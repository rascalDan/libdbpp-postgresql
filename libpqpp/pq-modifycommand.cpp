#include "pq-modifycommand.h"
#include "pq-error.h"
#include <stdlib.h>
#include "pq-connection.h"

PQ::ModifyCommand::ModifyCommand(const Connection * conn, const std::string & sql, unsigned int no) :
	DB::Command(sql),
	DB::ModifyCommand(sql),
	PQ::Command(conn, sql, no),
	prepared(false)
{
}

PQ::ModifyCommand::~ModifyCommand()
{
}

void
PQ::ModifyCommand::prepare() const
{
	if (!prepared) {
		std::string psql;
		psql.reserve(sql.length() + 20);
		prepareSql(psql, sql);
		c->checkResultFree(PQprepare(
					c->conn, stmntName.c_str(), psql.c_str(), values.size(), NULL), PGRES_COMMAND_OK);
		prepared = true;
	}
}

unsigned int
PQ::ModifyCommand::execute(bool anc)
{
	prepare();
	PGresult * res = PQexecPrepared(c->conn, stmntName.c_str(), values.size(), &values.front(), &lengths.front(), &formats.front(), 0);
	c->checkResult(res, PGRES_COMMAND_OK, PGRES_TUPLES_OK);
	unsigned int rows = atoi(PQcmdTuples(res));
	PQclear(res);
	if (rows == 0 && !anc) {
		throw DB::NoRowsAffected();
	}
	return rows;
}

