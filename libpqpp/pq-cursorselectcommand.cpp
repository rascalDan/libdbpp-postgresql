#include "pq-cursorselectcommand.h"
#include "pq-connection.h"
#include "pq-column.h"
#include "pq-error.h"
#include <compileTimeFormatter.h>

PQ::CursorSelectCommand::CursorSelectCommand(Connection * conn, const std::string & sql, unsigned int no) :
	DB::Command(sql),
	DB::SelectCommand(sql),
	PQ::Command(conn, sql, no),
	executed(false),
	txOpened(false),
	fTuples(35),
	s_declare(mkdeclare()),
	s_fetch(mkfetch()),
	s_close(mkclose())
{
}

PQ::CursorSelectCommand::~CursorSelectCommand()
{
	if (execRes) {
		PQclear(execRes);
	}
	if (executed && PQtransactionStatus(c->conn) != PQTRANS_INERROR) {
		c->checkResultFree((PQexec(c->conn, s_close.c_str())), PGRES_COMMAND_OK);
	}
	if (txOpened) {
		c->commitTx();
	}
}

AdHocFormatter(PQCursorSelectDeclare, "DECLARE %? CURSOR FOR %?");
std::string
PQ::CursorSelectCommand::mkdeclare() const
{
	std::string psql;
	prepareSql(psql, sql);
	return PQCursorSelectDeclare::get(stmntName, psql);
}

AdHocFormatter(PQCursorSelectFetch, "FETCH %? IN %?");
std::string
PQ::CursorSelectCommand::mkfetch() const
{
	return PQCursorSelectFetch::get(fTuples, stmntName);
}

AdHocFormatter(PQCursorSelectClose, "CLOSE %?");
std::string
PQ::CursorSelectCommand::mkclose() const
{
	return PQCursorSelectClose::get(stmntName);
}

void
PQ::CursorSelectCommand::execute()
{
	if (!executed) {
		if (!c->inTx()) {
			c->beginTx();
			txOpened = true;
		}
		execRes = c->checkResult(
				PQexecParams(c->conn, s_declare.c_str(), values.size(), NULL, &values.front(), &lengths.front(), NULL, 0),
				PGRES_COMMAND_OK);
		fetchTuples();
		unsigned int nFields = PQnfields(execRes);
		for (unsigned int f = 0; f < nFields; f += 1) {
			insertColumn(DB::ColumnPtr(new Column(this, f)));
		}
		executed = true;
	}
}

void
PQ::CursorSelectCommand::fetchTuples()
{
	if (execRes) {
		PQclear(execRes);
	}
	execRes = NULL;
	execRes = c->checkResult(PQexec(c->conn, s_fetch.c_str()), PGRES_TUPLES_OK);
	nTuples = PQntuples(execRes);
	tuple = -1;
}

bool
PQ::CursorSelectCommand::fetch()
{
	execute();
	if ((tuple >= (nTuples - 1)) && (nTuples == fTuples)) {
		fetchTuples();
	}
	if (tuple++ < (nTuples - 1)) {
		return true;
	}
	else {
		PQclear(PQexec(c->conn, s_close.c_str()));
		PQclear(execRes);
		execRes = NULL;
		executed = false;
		return false;
	}
}

