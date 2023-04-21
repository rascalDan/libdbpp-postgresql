#include "pq-modifycommand.h"
#include "command.h"
#include "modifycommand.h"
#include "pq-connection.h"
#include "pq-prepared.h"
#include <cstdlib>
#include <libpq-fe.h>
#include <vector>

PQ::ModifyCommand::ModifyCommand(Connection * conn, const std::string & sql, const DB::CommandOptionsCPtr & opts) :
	DB::Command(sql), DB::ModifyCommand(sql), PQ::PreparedStatement(conn, sql, opts)
{
}

unsigned int
PQ::ModifyCommand::execute(bool anc)
{
	PGresult * res = PQexecPrepared(
			c->conn, prepare(), static_cast<int>(values.size()), values.data(), lengths.data(), formats.data(), 0);
	c->checkResult(res, PGRES_COMMAND_OK, PGRES_TUPLES_OK);
	auto rows = atoi(PQcmdTuples(res));
	PQclear(res);
	if (rows == 0 && !anc) {
		throw DB::NoRowsAffected();
	}
	return static_cast<unsigned int>(rows);
}
