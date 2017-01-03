#include "pq-mock.h"
#include "pq-connection.h"
#include <compileTimeFormatter.h>
#include <modifycommand.h>
#include <selectcommand.h>
#include <selectcommandUtil.impl.h>
#include <boost/algorithm/string.hpp>

NAMEDFACTORY("postgresql", PQ::Mock, DB::MockDatabaseFactory);

namespace PQ {

Mock::Mock(const std::string & masterdb, const std::string & name, const std::vector<boost::filesystem::path> & ss) :
	MockServerDatabase(masterdb, name, "postgresql")
{
	CreateNewDatabase();
	PlaySchemaScripts(ss);
	SetTablesToUnlogged();
}

AdHocFormatter(MockConnStr, "user=postgres dbname=%?");
PQ::Connection *
Mock::openConnection() const
{
	return new Connection(MockConnStr::get(boost::algorithm::to_lower_copy(testDbName)));
}

AdHocFormatter(MockSetUnlogged, "ALTER TABLE %?.%? SET UNLOGGED");
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
			c->execute(MockSetUnlogged::get(t.value<0>(), t.value<1>()));
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
	auto t = master->modify("SELECT pg_terminate_backend(pid) FROM pg_stat_activity WHERE LOWER(datname) = LOWER(?)");
	t->bindParamS(0, testDbName);
	t->execute();
	MockServerDatabase::DropDatabase();
}

}

