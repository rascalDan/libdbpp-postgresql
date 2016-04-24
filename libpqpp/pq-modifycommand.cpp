#include "pq-modifycommand.h"
#include "pq-error.h"
#include <stdlib.h>
#include "pq-connection.h"

PQ::ModifyCommand::ModifyCommand(Connection * conn, const std::string & sql, unsigned int no) :
	DB::Command(sql),
	DB::ModifyCommand(sql),
	PQ::PreparedStatement(conn, sql, no)
{
}

PQ::ModifyCommand::~ModifyCommand()
{
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

