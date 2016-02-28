#include "pq-modifycommand.h"
#include "pq-error.h"
#include <stdlib.h>
#include "pq-connection.h"

PQ::ModifyCommand::ModifyCommand(Connection * conn, const std::string & sql, unsigned int no) :
	DB::Command(sql),
	DB::ModifyCommand(sql),
	PQ::Command(conn, sql, no),
	pstmt(nullptr)
{
}

PQ::ModifyCommand::~ModifyCommand()
{
}

const char *
PQ::ModifyCommand::prepare() const
{
	if (pstmt) {
		return pstmt;
	}
	auto hash(std::hash<std::string>()(sql));
	auto i = c->preparedStatements.find(hash);
	if (i != c->preparedStatements.end()) {
		return (pstmt = i->second.c_str());
	}
	std::string psql;
	psql.reserve(sql.length() + 20);
	prepareSql(psql, sql);
	c->checkResultFree(PQprepare(
				c->conn, stmntName.c_str(), psql.c_str(), values.size(), NULL), PGRES_COMMAND_OK);
	return (pstmt = c->preparedStatements.insert({hash, stmntName}).first->second.c_str());
}

unsigned int
PQ::ModifyCommand::execute(bool anc)
{
	PGresult * res = PQexecPrepared(c->conn, prepare(), values.size(), &values.front(), &lengths.front(), NULL, 0);
	c->checkResult(res, PGRES_COMMAND_OK, PGRES_TUPLES_OK);
	unsigned int rows = atoi(PQcmdTuples(res));
	PQclear(res);
	if (rows == 0 && !anc) {
		throw DB::NoRowsAffected();
	}
	return rows;
}

