#include "pq-cursorselectcommand.h"
#include "pq-connection.h"
#include "pq-error.h"
#include <compileTimeFormatter.h>

AdHocFormatter(PQCursorSelectDeclare, "DECLARE %? NO SCROLL CURSOR WITH HOLD FOR ");
AdHocFormatter(PQCursorSelectFetch, "FETCH %? IN %?");
AdHocFormatter(PQCursorSelectClose, "CLOSE %?");

PQ::CursorSelectCommand::CursorSelectCommand(Connection * conn, const std::string & sql, const PQ::CommandOptionsCPtr & pqco, const DB::CommandOptionsCPtr & opts) :
	DB::Command(sql),
	PQ::SelectBase(sql, pqco),
	PQ::Command(conn, sql, opts),
	executed(false),
	fTuples(pqco ? pqco->fetchTuples : 35),
	s_fetch(PQCursorSelectFetch::get(fTuples, stmntName)),
	s_close(PQCursorSelectClose::get(stmntName))
{
}

PQ::CursorSelectCommand::~CursorSelectCommand()
{
	if (executed && PQtransactionStatus(c->conn) != PQTRANS_INERROR) {
		c->checkResultFree((PQexec(c->conn, s_close.c_str())), PGRES_COMMAND_OK);
	}
}

std::string
PQ::CursorSelectCommand::mkdeclare() const
{
	std::stringstream psql;
	PQCursorSelectDeclare::write(psql, stmntName);
	prepareSql(psql, sql);
	return psql.str();
}

void
PQ::CursorSelectCommand::execute()
{
	if (!executed) {
		if (s_declare.empty()) {
			s_declare = mkdeclare();
		}
		c->checkResultFree(
				PQexecParams(c->conn, s_declare.c_str(), values.size(), nullptr, &values.front(), &lengths.front(), &formats.front(), binary),
				PGRES_COMMAND_OK);
		fetchTuples();
		createColumns(execRes);
		executed = true;
	}
}

void
PQ::CursorSelectCommand::fetchTuples()
{
	execRes = c->checkResult(PQexecParams(c->conn, s_fetch.c_str(), 0, nullptr, nullptr, nullptr, nullptr, binary), PGRES_TUPLES_OK);
	nTuples = PQntuples(execRes);
	tuple = -1;
}

bool
PQ::CursorSelectCommand::fetch()
{
	execute();
	if ((tuple + 1 >= nTuples) && (nTuples == fTuples)) {
		// Delete the previous result set
		PQclear(execRes);
		execRes = nullptr;
		fetchTuples();
	}
	if (++tuple < nTuples) {
		return true;
	}
	else {
		PQclear(PQexec(c->conn, s_close.c_str()));
		PQclear(execRes);
		execRes = nullptr;
		executed = false;
		return false;
	}
}

