#include "selectcommand.h"
#include "connection.h"
#include "column.h"
#include "error.h"

PQ::SelectCommand::SelectCommand(const Connection * conn, const std::string & sql, unsigned int no) :
	DB::Command(sql),
	DB::SelectCommand(sql),
	PQ::Command(conn, sql, no),
	executed(false),
	nTuples(0),
	tuple(0),
	execRes(NULL)
{
}

PQ::SelectCommand::~SelectCommand()
{
	if (executed) {
		c->commitTx();
		PQclear(PQexec(c->conn, ("CLOSE " + stmntName).c_str()));
		if (execRes) {
			PQclear(execRes);
		}
	}
	for (unsigned int f = 0; f < fields.size(); f += 1) {
		delete fields[f];
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
		c->checkResultFree(
				PQexecParams(c->conn, psql.c_str(), values.size(), NULL, &values.front(), &lengths.front(), &formats.front(), 0),
				PGRES_COMMAND_OK);
		executed = true;
	}
}

bool
PQ::SelectCommand::fetch()
{
	execute();
	if (tuple >= (nTuples - 1)) {
		if (execRes) {
			PQclear(execRes);
		}
		execRes = NULL;
		execRes = c->checkResult(PQexec(c->conn, ("FETCH 35 IN " + stmntName).c_str()), PGRES_TUPLES_OK);
		nTuples = PQntuples(execRes);
		tuple = -1;
	}
	if (fields.empty()) {
		unsigned int nFields = PQnfields(execRes);
		fields.resize(nFields);
		for (unsigned int f = 0; f < nFields; f += 1) {
			Column * c = new Column(this, f);
			fields[f] = c;
			fieldsName[c->name] = c;
		}
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

const DB::Column&
PQ::SelectCommand::operator[](unsigned int n) const
{
	if (n < fields.size()) {
		return *fields[n];
	}
	throw Error();
}

const DB::Column&
PQ::SelectCommand::operator[](const Glib::ustring & n) const
{
	std::map<Glib::ustring, Column *>::const_iterator i = fieldsName.find(n);
	if (i != fieldsName.end()) {
		return *i->second;
	}
	throw Error();
}

unsigned int
PQ::SelectCommand::getOrdinal(const Glib::ustring & n) const
{
	std::map<Glib::ustring, Column *>::const_iterator i = fieldsName.find(n);
	if (i != fieldsName.end()) {
		return i->second->colNo;
	}
	throw Error();
}

unsigned int
PQ::SelectCommand::columnCount() const
{
	return fields.size();
}

