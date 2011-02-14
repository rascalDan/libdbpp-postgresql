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
	if (execRes) {
		PQclear(execRes);
	}
	for (unsigned int f = 0; f < fields.size(); f += 1) {
		delete fields[f];
	}
}

void
PQ::SelectCommand::execute()
{
	if (!executed) {
		prepare();
		execRes = PQexecPrepared(c->conn, stmntName.c_str(), values.size(), &values.front(), &lengths.front(), &formats.front(), 0);
		c->checkResult(execRes, PGRES_TUPLES_OK);
		unsigned int nFields = PQnfields(execRes);
		fields.resize(nFields);
		for (unsigned int f = 0; f < nFields; f += 1) {
			Column * c = new Column(this, f);
			fields[f] = c;
			fieldsName[c->name] = c;
		}
        nTuples = PQntuples(execRes);
		tuple = -1;
		executed = true;
	}
}

bool
PQ::SelectCommand::fetch()
{
	execute();
	if (tuple++ < (nTuples - 1)) {
		return true;
	}
	else {
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

