#include "pq-cursorselectcommand.h"
#include "command.h"
#include "pq-command.h"
#include "pq-connection.h"
#include "pq-selectbase.h"
#include <compileTimeFormatter.h>
#include <libpq-fe.h>
#include <type_traits>
#include <utility>
#include <vector>

AdHocFormatter(PQCursorSelectDeclare, "DECLARE %? NO SCROLL CURSOR WITH HOLD FOR ");
AdHocFormatter(PQCursorSelectFetch, "FETCH %? IN %?");
AdHocFormatter(PQCursorSelectClose, "CLOSE %?");

PQ::CursorSelectCommand::CursorSelectCommand(Connection * conn, const std::string & sql,
		const PQ::CommandOptionsCPtr & pqco, const DB::CommandOptionsCPtr & opts) :
	DB::Command(sql),
	PQ::SelectBase(sql, pqco), PQ::Command(conn, sql, opts), executed(false), fTuples(pqco ? pqco->fetchTuples : 35),
	s_fetch(PQCursorSelectFetch::get(fTuples, stmntName)), s_close(PQCursorSelectClose::get(stmntName))
{
}

PQ::CursorSelectCommand::~CursorSelectCommand()
{
	if (executed && PQtransactionStatus(c->conn) != PQTRANS_INERROR) {
		c->checkResult(PQexec(c->conn, s_close.c_str()), PGRES_COMMAND_OK);
	}
}

std::string
PQ::CursorSelectCommand::mkdeclare() const
{
	std::stringstream psql;
	PQCursorSelectDeclare::write(psql, stmntName);
	prepareSql(psql, sql);
	return std::move(psql).str();
}

void
PQ::CursorSelectCommand::execute()
{
	if (!executed) {
		if (s_declare.empty()) {
			s_declare = mkdeclare();
		}
		c->checkResult(PQexecParams(c->conn, s_declare.c_str(), static_cast<int>(values.size()), nullptr, values.data(),
							   lengths.data(), formats.data(), binary),
				PGRES_COMMAND_OK);
		fetchTuples();
		createColumns();
		executed = true;
	}
}

void
PQ::CursorSelectCommand::fetchTuples()
{
	execRes = c->checkResult(
			PQexecParams(c->conn, s_fetch.c_str(), 0, nullptr, nullptr, nullptr, nullptr, binary), PGRES_TUPLES_OK);
	nTuples = static_cast<decltype(nTuples)>(PQntuples(execRes.get()));
	tuple = static_cast<decltype(tuple)>(-1);
}

bool
PQ::CursorSelectCommand::fetch()
{
	execute();
	if ((tuple + 1 >= nTuples) && (nTuples == fTuples)) {
		// Delete the previous result set
		execRes.reset();
		fetchTuples();
	}
	if (++tuple < nTuples) {
		return true;
	}
	else {
		PQclear(PQexec(c->conn, s_close.c_str()));
		execRes.reset();
		executed = false;
		return false;
	}
}
