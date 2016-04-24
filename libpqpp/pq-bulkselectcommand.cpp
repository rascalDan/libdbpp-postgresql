#include "pq-bulkselectcommand.h"
#include "pq-connection.h"
#include "pq-column.h"
#include "pq-error.h"

PQ::BulkSelectCommand::BulkSelectCommand(Connection * conn, const std::string & sql, unsigned int no) :
	DB::Command(sql),
	DB::SelectCommand(sql),
	PQ::Command(conn, sql, no),
	executed(false)
{
	prepareSql(preparedSql, sql);
}

PQ::BulkSelectCommand::~BulkSelectCommand()
{
	if (execRes) {
		PQclear(execRes);
	}
}

void
PQ::BulkSelectCommand::execute()
{
	if (!executed) {
		execRes = c->checkResult(
				PQexecParams(c->conn, preparedSql.c_str(), values.size(), NULL, &values.front(), &lengths.front(), NULL, 0),
				PGRES_TUPLES_OK);
		nTuples = PQntuples(execRes);
		tuple = -1;
		unsigned int nFields = PQnfields(execRes);
		for (unsigned int f = 0; f < nFields; f += 1) {
			insertColumn(DB::ColumnPtr(new Column(this, f)));
		}
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

