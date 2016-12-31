#include "pq-connection.h"
#include "pq-error.h"
#include "pq-bulkselectcommand.h"
#include "pq-cursorselectcommand.h"
#include "pq-modifycommand.h"
#include <unistd.h>
#include <poll.h>
#include <boost/assert.hpp>
#include <compileTimeFormatter.h>

NAMEDFACTORY("postgresql", PQ::Connection, DB::ConnectionFactory);

static void setup()  __attribute__((constructor(101)));
static void setup()
{
	BOOST_ASSERT(PQisthreadsafe() == 1);
	PQinitOpenSSL(1, 0);
}

static void
noNoticeProcessor(void *, const char *)
{
}

PQ::ConnectionError::ConnectionError(const PGconn * conn) :
	PQ::Error(conn)
{
}

PQ::Connection::Connection(const std::string & info) :
	conn(PQconnectdb(info.c_str())),
	pstmntNo(0)
{
	if (PQstatus(conn) != CONNECTION_OK) {
		ConnectionError ce(conn);
		PQfinish(conn);
		throw ce;
	}
	PQsetNoticeProcessor(conn, noNoticeProcessor, NULL);
}

PQ::Connection::~Connection()
{
	PQfinish(conn);
}

void
PQ::Connection::beginTxInt()
{
	checkResultFree(PQexec(conn, "BEGIN"), PGRES_COMMAND_OK);
}

void
PQ::Connection::commitTxInt()
{
	checkResultFree(PQexec(conn, "COMMIT"), PGRES_COMMAND_OK);
}

void
PQ::Connection::rollbackTxInt()
{
	checkResultFree(PQexec(conn, "ROLLBACK"), PGRES_COMMAND_OK);
}

void
PQ::Connection::execute(const std::string & sql)
{
	checkResultFree(PQexec(conn, sql.c_str()), PGRES_COMMAND_OK, PGRES_TUPLES_OK);
}

DB::BulkDeleteStyle
PQ::Connection::bulkDeleteStyle() const
{
	return DB::BulkDeleteUsingSubSelect;
}

DB::BulkUpdateStyle
PQ::Connection::bulkUpdateStyle() const
{
	return DB::BulkUpdateUsingFromSrc;
}

void
PQ::Connection::ping() const
{
	struct pollfd fd { PQsocket(conn), POLLRDHUP | POLLERR | POLLHUP | POLLNVAL, 0 };
	if (PQstatus(conn) != CONNECTION_OK || poll(&fd, 1, 0)) {
		if (inTx()) {
			throw ConnectionError(conn);
		}
		PQreset(conn);
		if (PQstatus(conn) != CONNECTION_OK) {
			throw ConnectionError(conn);
		}
	}
}


DB::SelectCommand *
PQ::Connection::newSelectCommand(const std::string & sql)
{
	// Yes, this is a hack
	if (sql.find("libdbpp:no-cursor") != (std::string::size_type)-1) {
		return new BulkSelectCommand(this, sql, pstmntNo++);
	}
	return new CursorSelectCommand(this, sql, pstmntNo++);
}

DB::ModifyCommand *
PQ::Connection::newModifyCommand(const std::string & sql)
{
	return new ModifyCommand(this, sql, pstmntNo++);
}

bool
PQ::Connection::checkResultInt(PGresult * res, int expected, int alt)
{
	return (PQresultStatus(res) == expected) || (alt != -1 && (PQresultStatus(res) == alt));
}

PGresult *
PQ::Connection::checkResult(PGresult * res, int expected, int alt) const
{
	if (!checkResultInt(res, expected, alt)) {
		PQclear(res);
		throw Error(conn);
	}
	return res;
}

void
PQ::Connection::checkResultFree(PGresult * res, int expected, int alt) const
{
	if (!checkResultInt(res, expected, alt)) {
		PQclear(res);
		throw Error(conn);
	}
	PQclear(res);
}

AdHocFormatter(PQConnectionCopyFrom, "COPY %? FROM STDIN %?");
void
PQ::Connection::beginBulkUpload(const char * table, const char * extra)
{
	checkResultFree(PQexec(conn, PQConnectionCopyFrom::get(table, extra).c_str()), PGRES_COPY_IN);
}

void
PQ::Connection::endBulkUpload(const char * msg)
{
	switch (PQputCopyEnd(conn, msg)) {
		case 0:// block
			sleep(1);
			endBulkUpload(msg);
			return;
		case 1:// success
			checkResultFree(PQgetResult(conn), PGRES_COMMAND_OK);
			return;
		default:// -1 is error
			throw Error(conn);
	}
}

size_t
PQ::Connection::bulkUploadData(const char * data, size_t len) const
{
	switch (PQputCopyData(conn, data, len)) {
		case 0:// block
			sleep(1);
			return bulkUploadData(data, len);
		case 1:// success
			return len;
		default:// -1 is error
			throw Error(conn);
	}
}

int64_t
PQ::Connection::insertId()
{
	CursorSelectCommand getId(this, "SELECT lastval()", pstmntNo++);
	int64_t id = -1;
	while (getId.fetch()) {
		getId[0] >> id;
	}
	return id;
}

