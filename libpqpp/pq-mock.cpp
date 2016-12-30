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
	auto s = c->select(R"SQL( 
SELECT n.nspname, c.relname
FROM pg_class c, pg_namespace n
WHERE c.relkind = 'r'
AND n.nspname not in (?, ?)
AND c.relpersistence = 'p'
AND NOT EXISTS (
	SELECT from pg_constraint fk, pg_class ck
	WHERE fk.contype = 'f'
	AND fk.confrelid = c.oid
	AND fk.conrelid = ck.oid
	AND ck.oid != c.oid
	AND ck.relpersistence = 'p')
AND n.oid = c.relnamespace
ORDER BY 1, 2)SQL");
	s->bindParamS(0, "pg_catalog");
	s->bindParamS(1, "information_schema");
	unsigned int n = 0;
	do {
		n = 0;
		for (const auto & t : s->as<std::string, std::string>()) {
			c->execute("ALTER TABLE " + t.value<0>() + "." + t.value<1>() + " SET UNLOGGED");
			n += 1;
		}
	} while(n);
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

