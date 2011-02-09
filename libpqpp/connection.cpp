#include "connection.h"
#include "error.h"
#include "selectcommand.h"
#include "modifycommand.h"

PQ::Connection::Connection(const std::string & info) :
	conn(PQconnectdb(info.c_str())),
	txDepth(0),
	pstmntNo(0)
{
	if (PQstatus(conn) != CONNECTION_OK) {
		throw ConnectionError();
	}
}

PQ::Connection::~Connection()
{
	PQfinish(conn);
}

int
PQ::Connection::beginTx() const
{
	if (txDepth == 0) {
		checkResultFree(PQexec(conn, "BEGIN"), PGRES_COMMAND_OK, __PRETTY_FUNCTION__);
	}
	return ++txDepth;
}

int
PQ::Connection::commitTx() const
{
	if (--txDepth == 0) {
		checkResultFree(PQexec(conn, "COMMIT"), PGRES_COMMAND_OK, __PRETTY_FUNCTION__);
	}
	return txDepth;
}

int
PQ::Connection::rollbackTx() const
{
	if (--txDepth == 0) {
		checkResultFree(PQexec(conn, "ROLLBACK"), PGRES_COMMAND_OK, __PRETTY_FUNCTION__);
	}
	return txDepth;
}

bool
PQ::Connection::inTx() const
{
	return txDepth;
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
PQ::Connection::checkResultInt(PGresult * res, int expected)
{
	return (PQresultStatus(res) == expected);
}

void
PQ::Connection::checkResult(PGresult * res, int expected, const char * doing) const
{
	if (!checkResultInt(res, expected)) {
		PQclear(res);
		throw Error(PQerrorMessage(conn));
	}
}

void
PQ::Connection::checkResultFree(PGresult * res, int expected, const char * doing) const
{
	if (!checkResultInt(res, expected)) {
		PQclear(res);
		throw Error(PQerrorMessage(conn));
	}
	PQclear(res);
}

