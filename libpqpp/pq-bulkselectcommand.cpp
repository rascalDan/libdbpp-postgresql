#include "pq-bulkselectcommand.h"
#include "command.h"
#include "libpq-fe.h"
#include "pq-command.h"
#include "pq-connection.h"
#include "pq-prepared.h"
#include "pq-selectbase.h"
#include <vector>

PQ::BulkSelectCommand::BulkSelectCommand(Connection * conn, const std::string & sql,
		const PQ::CommandOptionsCPtr & pqco, const DB::CommandOptionsCPtr & opts) :
	DB::Command(sql),
	PQ::SelectBase(sql, pqco), PQ::PreparedStatement(conn, sql, opts)
{
}

void
PQ::BulkSelectCommand::execute()
{
	if (!execRes) {
		execRes = c->checkResult(PQexecPrepared(c->conn, prepare(), static_cast<int>(values.size()), values.data(),
										 lengths.data(), formats.data(), binary),
				PGRES_TUPLES_OK);
		nTuples = static_cast<decltype(nTuples)>(PQntuples(execRes.get()));
		tuple = static_cast<decltype(tuple)>(-1);
		createColumns();
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
		execRes.reset();
		return false;
	}
}
