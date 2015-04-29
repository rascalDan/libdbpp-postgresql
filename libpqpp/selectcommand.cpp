#include "selectcommand.h"
#include "connection.h"
#include "column.h"
#include "error.h"

PQ::SelectCommand::SelectCommand(const Connection * conn, const std::string & sql, unsigned int no) :
	DB::Command(sql),
	DB::SelectCommand(sql),
	PQ::Command(conn, sql, no),
	executed(false),
	txOpened(false),
	nTuples(0),
	tuple(0),
	execRes(NULL)
{
}

PQ::SelectCommand::~SelectCommand()
{
	if (txOpened) {
		c->commitTx();
	}
	if (executed) {
		PQclear(PQexec(c->conn, ("CLOSE " + stmntName).c_str()));
		if (execRes) {
			PQclear(execRes);
		}
	}
}

void
PQ::SelectCommand::execute()
{
	if (!executed) {
		std::string psql;
		psql.reserve(sql.length() + 40);
		char buf[4];
		int p = 1;
		bool inquote = false;
		psql += "DECLARE ";
		psql += stmntName;
		psql += " CURSOR FOR ";
		for(std::string::const_iterator i = sql.begin(); i != sql.end(); ++i) {
			if (*i == '?' && !inquote) {
				snprintf(buf, 4, "$%d", p++);
				psql += buf;
			}
			else if (*i == '\'') {
				inquote = !inquote;
				psql += *i;
			}
			else {
				psql += *i;
			}
		}
		c->beginTx();
		txOpened = true;
		execRes = c->checkResult(
				PQexecParams(c->conn, psql.c_str(), values.size(), NULL, &values.front(), &lengths.front(), &formats.front(), 0),
				PGRES_COMMAND_OK);
		fetchTuples();
		unsigned int nFields = PQnfields(execRes);
		for (unsigned int f = 0; f < nFields; f += 1) {
			columns.insert(DB::ColumnPtr(new Column(this, f)));
		}
		executed = true;
	}
}

void
PQ::SelectCommand::fetchTuples()
{
	if (execRes) {
		PQclear(execRes);
	}
	execRes = NULL;
	execRes = c->checkResult(PQexec(c->conn, ("FETCH 35 IN " + stmntName).c_str()), PGRES_TUPLES_OK);
	nTuples = PQntuples(execRes);
	tuple = -1;
}

bool
PQ::SelectCommand::fetch()
{
	execute();
	if (tuple >= (nTuples - 1)) {
		fetchTuples();
	}
	if (tuple++ < (nTuples - 1)) {
		return true;
	}
	else {
		PQclear(PQexec(c->conn, ("CLOSE " + stmntName).c_str()));
		PQclear(execRes);
		executed = false;
		return false;
	}
}

