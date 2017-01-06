#include "pq-bulkselectcommand.h"
#include "pq-connection.h"
#include "pq-column.h"
#include "pq-error.h"

PQ::BulkSelectCommand::BulkSelectCommand(Connection * conn, const std::string & sql, const DB::CommandOptions * opts) :
	DB::Command(sql),
	PQ::SelectBase(sql),
	PQ::PreparedStatement(conn, sql, opts),
	executed(false)
{
}

void
PQ::BulkSelectCommand::execute()
{
	if (!executed) {
		execRes = c->checkResult(
				PQexecPrepared(c->conn, prepare(), values.size(), &values.front(), &lengths.front(), NULL, 0),
				PGRES_TUPLES_OK);
		nTuples = PQntuples(execRes);
		tuple = -1;
		createColumns(execRes);
		executed = true;
	}
}

bool
PQ::BulkSelectCommand::fetch()
{
	execute();
	if (++tuple < nTuples) {
		return true;
	}
	else {
		PQclear(execRes);
		execRes = NULL;
		executed = false;
		return false;
	}
}

