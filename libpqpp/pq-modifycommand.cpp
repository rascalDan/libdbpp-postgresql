#include "pq-modifycommand.h"
#include "pq-error.h"
#include <stdlib.h>
#include "pq-connection.h"

PQ::ModifyCommand::ModifyCommand(Connection * conn, const std::string & sql, unsigned int no) :
	DB::Command(sql),
	DB::ModifyCommand(sql),
	PQ::Command(conn, sql, no),
	hash(std::hash<std::string>()(sql))
{
}

PQ::ModifyCommand::~ModifyCommand()
{
}

PQ::Connection::PreparedStatements::const_iterator
PQ::ModifyCommand::prepare() const
{
	auto i = c->preparedStatements.find(hash);
	if (i != c->preparedStatements.end()) {
		return i;
	}
	std::string psql;
	psql.reserve(sql.length() + 20);
	prepareSql(psql, sql);
	c->checkResultFree(PQprepare(
				c->conn, stmntName.c_str(), psql.c_str(), values.size(), NULL), PGRES_COMMAND_OK);
	return c->preparedStatements.insert({hash, stmntName}).first;
}

unsigned int
PQ::ModifyCommand::execute(bool anc)
{
	PGresult * res = PQexecPrepared(c->conn, prepare()->second.c_str(), values.size(), &values.front(), &lengths.front(), NULL, 0);
	c->checkResult(res, PGRES_COMMAND_OK, PGRES_TUPLES_OK);
	unsigned int rows = atoi(PQcmdTuples(res));
	PQclear(res);
	if (rows == 0 && !anc) {
		throw DB::NoRowsAffected();
	}
	return rows;
}

