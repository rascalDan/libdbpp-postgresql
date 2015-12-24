#include "pq-mock.h"
#include "pq-connection.h"
#include <buffer.h>

namespace PQ {

Mock::Mock(const std::string & masterdb, const std::string & name, const std::vector<boost::filesystem::path> & ss) :
	MockServerDatabase(masterdb, name, "postgresql")
{
	CreateNewDatabase();
	PlaySchemaScripts(ss);
}

DB::Connection *
Mock::openConnection() const
{
	return new Connection(stringbf("user=postgres dbname=%s", testDbName));
}

Mock::~Mock()
{
	DropDatabase();
}

void
Mock::DropDatabase() const
{
	master->execute("SELECT pg_terminate_backend(pid) FROM pg_stat_activity WHERE datname = '" + testDbName + "'");
	MockServerDatabase::DropDatabase();
}

}

