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
	const auto res = c->checkResult(PQexecPrepared(c->conn, prepare(), static_cast<int>(values.size()), values.data(),
											lengths.data(), formats.data(), 0),
			PGRES_COMMAND_OK, PGRES_TUPLES_OK);
	auto rows = atoi(PQcmdTuples(res.get()));
	if (rows == 0 && !anc) {
		throw DB::NoRowsAffected();
	}
	return static_cast<unsigned int>(rows);
}
