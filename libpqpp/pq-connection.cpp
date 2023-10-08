#include "pq-connection.h"
#include "column.h"
#include "command.h"
#include "connection.h"
#include "pq-bulkselectcommand.h"
#include "pq-command.h"
#include "pq-cursorselectcommand.h"
#include "pq-error.h"
#include "pq-modifycommand.h"
#include "selectcommand.h"
#include <boost/assert.hpp>
#include <compileTimeFormatter.h>
#include <factory.h>
#include <libpq-fe.h>
#include <memory>
#include <poll.h>
#include <unistd.h>

NAMEDFACTORY("postgresql", PQ::Connection, DB::ConnectionFactory)

static void setup() __attribute__((constructor(101)));

static void
setup()
{
	// NOLINTNEXTLINE(hicpp-no-array-decay)
	BOOST_ASSERT(PQisthreadsafe() == 1);
	PQinitOpenSSL(1, 0);
}

static void
noNoticeProcessor(void *, const char *)
{
}

// NOLINTNEXTLINE(bugprone-throw-keyword-missing)
PQ::ConnectionError::ConnectionError(const PGconn * conn) : PQ::Error(conn) { }

PQ::Connection::Connection(const std::string & info) : conn(PQconnectdb(info.c_str()))
{
	if (PQstatus(conn) != CONNECTION_OK) {
		auto dc = std::unique_ptr<PGconn, decltype(&PQfinish)>(conn, &PQfinish);
		throw ConnectionError(dc.get());
	}
	PQsetNoticeProcessor(conn, noNoticeProcessor, nullptr);
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
PQ::Connection::execute(const std::string & sql, const DB::CommandOptionsCPtr &)
{
	checkResultFree(PQexec(conn, sql.c_str()), PGRES_COMMAND_OK, PGRES_TUPLES_OK);
}

DB::BulkDeleteStyle
PQ::Connection::bulkDeleteStyle() const
{
	return DB::BulkDeleteStyle::UsingSubSelect;
}

DB::BulkUpdateStyle
PQ::Connection::bulkUpdateStyle() const
{
	return DB::BulkUpdateStyle::UsingFromSrc;
}

void
PQ::Connection::ping() const
{
	struct pollfd fd {
		// NOLINTNEXTLINE(hicpp-signed-bitwise)
		PQsocket(conn), POLLRDHUP | POLLERR | POLLHUP | POLLNVAL, 0
	};

	if (PQstatus(conn) != CONNECTION_OK || poll(&fd, 1, 0)) {
		if (inTx()) {
			throw ConnectionError(conn);
		}
		preparedStatements.clear();
		PQreset(conn);
		if (PQstatus(conn) != CONNECTION_OK) {
			throw ConnectionError(conn);
		}
	}
}

DB::SelectCommandPtr
PQ::Connection::select(const std::string & sql, const DB::CommandOptionsCPtr & opts)
{
	auto pqco = std::dynamic_pointer_cast<const CommandOptions>(opts);
	if (pqco && !pqco->useCursor) {
		return std::make_shared<BulkSelectCommand>(this, sql, pqco, opts);
	}
	return std::make_shared<CursorSelectCommand>(this, sql, pqco, opts);
}

DB::ModifyCommandPtr
PQ::Connection::modify(const std::string & sql, const DB::CommandOptionsCPtr & opts)
{
	return std::make_shared<ModifyCommand>(this, sql, opts);
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
	int rc;
	while (!(rc = PQputCopyEnd(conn, msg))) {
		sleep(1);
	}
	if (rc != 1) {
		throw Error(conn);
	}
	checkResultFree(PQgetResult(conn), PGRES_COMMAND_OK);
}

size_t
PQ::Connection::bulkUploadData(const char * data, size_t len) const
{
	int rc;
	while (!(rc = PQputCopyData(conn, data, static_cast<int>(len)))) {
		sleep(1);
	}
	if (rc != 1) {
		throw Error(conn);
	}
	return len;
}

static const std::string selectLastVal("SELECT lastval()");
static const DB::CommandOptionsCPtr selectLastValOpts
		= std::make_shared<DB::CommandOptions>(std::hash<std::string>()(selectLastVal));

int64_t
PQ::Connection::insertId()
{
	BulkSelectCommand getId(this, selectLastVal, nullptr, selectLastValOpts);
	int64_t id = -1;
	while (getId.fetch()) {
		getId[0] >> id;
	}
	return id;
}

int
PQ::Connection::serverVersion() const
{
	return PQserverVersion(conn);
}
