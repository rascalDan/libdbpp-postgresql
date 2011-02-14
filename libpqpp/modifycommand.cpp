#include "modifycommand.h"
#include "error.h"
#include <stdlib.h>
#include "connection.h"

PQ::ModifyCommand::ModifyCommand(const Connection * conn, const std::string & sql, unsigned int no) :
	DB::Command(sql),
	DB::ModifyCommand(sql),
	PQ::Command(conn, sql, no)
{
}

PQ::ModifyCommand::~ModifyCommand()
{
}

unsigned int
PQ::ModifyCommand::execute(bool anc)
{
	prepare();
	PGresult * res = PQexecPrepared(c->conn, stmntName.c_str(), values.size(), &values.front(), &lengths.front(), &formats.front(), 0);
	c->checkResult(res, PGRES_COMMAND_OK);
	unsigned int rows = atoi(PQcmdTuples(res));
	PQclear(res);
	if (rows == 0 && !anc) {
		throw Error("No rows affected");
	}
	return rows;
}

