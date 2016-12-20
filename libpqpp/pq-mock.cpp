#include "pq-mock.h"
#include "pq-connection.h"
#include <buffer.h>
#include <selectcommand.h>
#include <selectcommandUtil.impl.h>

namespace PQ {

Mock::Mock(const std::string & masterdb, const std::string & name, const std::vector<boost::filesystem::path> & ss) :
	MockServerDatabase(masterdb, name, "postgresql")
{
	CreateNewDatabase();
	PlaySchemaScripts(ss);
	SetTablesToUnlogged();
}

DB::Connection *
Mock::openConnection() const
{
	return new Connection(stringbf("user=postgres dbname=%s", testDbName));
}

void
Mock::SetTablesToUnlogged() const
{
	auto c = DB::ConnectionPtr(openConnection());
	auto s = c->select("SELECT schemaname, tablename FROM pg_catalog.pg_tables WHERE schemaname NOT IN (?, ?)");
	s->bindParamS(0, "pg_catalog");
	s->bindParamS(1, "information_schema");
	for (const auto & t : s->as<std::string, std::string>()) {
		c->execute("ALTER TABLE " + t.value<0>() + "." + t.value<1>() + " SET UNLOGGED");
	}
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

