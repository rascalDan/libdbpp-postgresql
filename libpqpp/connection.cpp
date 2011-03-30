#include "connection.h"
#include "error.h"
#include "selectcommand.h"
#include "modifycommand.h"

static void
noNoticeProcessor(void *, const char *)
{
}

PQ::Connection::Connection(const std::string & info) :
	conn(PQconnectdb(info.c_str())),
	txDepth(0),
	pstmntNo(0),
	rolledback(false)
{
	if (PQstatus(conn) != CONNECTION_OK) {
		throw ConnectionError();
	}
	PQsetNoticeProcessor(conn, noNoticeProcessor, NULL);
}

PQ::Connection::~Connection()
{
	PQfinish(conn);
}

void
PQ::Connection::finish() const
{
	if (txDepth != 0) {
		rollbackTx();
		throw Error("Transaction still open");
	}
}

int
PQ::Connection::beginTx() const
{
	if (txDepth == 0) {
		checkResultFree(PQexec(conn, "BEGIN"), PGRES_COMMAND_OK);
		rolledback = false;
	}
	return ++txDepth;
}

int
PQ::Connection::commitTx() const
{
	if (rolledback) {
		return rollbackTx();
	}
	if (--txDepth == 0) {
		checkResultFree(PQexec(conn, "COMMIT"), PGRES_COMMAND_OK);
	}
	return txDepth;
}

int
PQ::Connection::rollbackTx() const
{
	if (--txDepth == 0) {
		checkResultFree(PQexec(conn, "ROLLBACK"), PGRES_COMMAND_OK);
	}
	else {
		rolledback = true;
	}
	return txDepth;
}

bool
PQ::Connection::inTx() const
{
	return txDepth;
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
}


DB::SelectCommand *
PQ::Connection::newSelectCommand(const std::string & sql) const
{
	return new SelectCommand(this, sql, pstmntNo++);
}

DB::ModifyCommand *
PQ::Connection::newModifyCommand(const std::string & sql) const
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
		throw Error(PQerrorMessage(conn));
	}
	return res;
}

void
PQ::Connection::checkResultFree(PGresult * res, int expected, int alt) const
{
	if (!checkResultInt(res, expected, alt)) {
		PQclear(res);
		throw Error(PQerrorMessage(conn));
	}
	PQclear(res);
}

