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
	fTuples(35),
	execRes(NULL),
	s_declare(mkdeclare()),
	s_fetch(mkfetch()),
	s_close(mkclose())
{
}

PQ::SelectCommand::~SelectCommand()
{
	if (txOpened) {
		c->commitTx();
	}
	if (executed) {
		PQclear(PQexec(c->conn, s_close.c_str()));
		if (execRes) {
			PQclear(execRes);
		}
	}
}

std::string
PQ::SelectCommand::mkdeclare() const
{
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
	return psql;
}

std::string
PQ::SelectCommand::mkfetch() const
{
	char buf[BUFSIZ];
	snprintf(buf, sizeof(buf), "FETCH %d IN %s", fTuples, stmntName.c_str());
	return buf;
}

std::string
PQ::SelectCommand::mkclose() const
{
	char buf[BUFSIZ];
	snprintf(buf, sizeof(buf), "CLOSE %s", stmntName.c_str());
	return buf;
}

void
PQ::SelectCommand::execute()
{
	if (!executed) {
		if (!txOpened) {
			c->beginTx();
			txOpened = true;
		}
		execRes = c->checkResult(
				PQexecParams(c->conn, s_declare.c_str(), values.size(), NULL, &values.front(), &lengths.front(), &formats.front(), 0),
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
PQ::SelectCommand::fetchTuples()
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
PQ::SelectCommand::fetch()
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

