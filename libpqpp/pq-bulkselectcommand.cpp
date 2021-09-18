#include "pq-bulkselectcommand.h"
#include "pq-column.h"
#include "pq-connection.h"
#include "pq-error.h"

PQ::BulkSelectCommand::BulkSelectCommand(Connection * conn, const std::string & sql,
		const PQ::CommandOptionsCPtr & pqco, const DB::CommandOptionsCPtr & opts) :
	DB::Command(sql),
	PQ::SelectBase(sql, pqco), PQ::PreparedStatement(conn, sql, opts), executed(false)
{
}

void
PQ::BulkSelectCommand::execute()
{
	if (!executed) {
		execRes = c->checkResult(PQexecPrepared(c->conn, prepare(), static_cast<int>(values.size()), &values.front(),
										 &lengths.front(), &formats.front(), binary),
				PGRES_TUPLES_OK);
		nTuples = static_cast<decltype(nTuples)>(PQntuples(execRes));
		tuple = static_cast<decltype(tuple)>(-1);
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
		execRes = nullptr;
		executed = false;
		return false;
	}
}
