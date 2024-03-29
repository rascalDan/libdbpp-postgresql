#include "pq-mock.h"
#include "mockDatabase.h"
#include "pq-connection.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <compileTimeFormatter.h>
#include <factory.h>
#include <memory>
#include <modifycommand.h>
#include <selectcommand.h>
#include <selectcommandUtil.impl.h>
// IWYU pragma: no_include <boost/iterator/iterator_facade.hpp>

NAMEDFACTORY("postgresql", PQ::Mock, DB::MockDatabaseFactory)

namespace PQ {

	Mock::Mock(const std::string & masterdb, const std::string & name, const std::vector<std::filesystem::path> & ss) :
		MockServerDatabase(masterdb, name, "postgresql"),
		tablespacePath(std::filesystem::temp_directory_path() / testDbName),
		serverVersion(std::static_pointer_cast<Connection>(master)->serverVersion())
	{
		try {
			CreateNewDatabase();
			PlaySchemaScripts(ss);
			SetTablesToUnlogged();
		}
		catch (...) {
			DropDatabase();
			throw;
		}
	}

	AdHocFormatter(MockConnStr, "user=postgres dbname=%?");

	DB::ConnectionPtr
	Mock::openConnection() const
	{
		return std::make_shared<Connection>(MockConnStr::get(boost::algorithm::to_lower_copy(testDbName)));
	}

	AdHocFormatter(MockSetUnlogged, "ALTER TABLE %?.%? SET UNLOGGED");

	void
	Mock::SetTablesToUnlogged() const
	{
		if (!hasUnloggedTables()) {
			return;
		}
		auto s = master->select(R"SQL(
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
		while ([&s, this]() {
			unsigned int n = 0;
			for (const auto [nspname, relname] : s->as<std::string, std::string>()) {
				master->execute(MockSetUnlogged::get(nspname, relname));
				n += 1;
			}
			return n;
		}()) {
			;
		}
	}

	Mock::~Mock()
	{
		Mock::DropDatabase();
	}

	bool
	Mock::hasUnloggedTables() const
	{
		// v9.5 server required for unlogged tables
		return (serverVersion >= 90500);
	}

	bool
	Mock::hasCopyToProgram() const
	{
		// v9.3 server required to use COPY ... TO PROGRAM ...
		return (serverVersion >= 90300);
	}

	AdHocFormatter(MockCreateTablespaceDir, "COPY (SELECT '%?') TO PROGRAM 'xargs mkdir -p'");
	AdHocFormatter(MockCreateTablespace, "CREATE TABLESPACE %? LOCATION '%?'");
	AdHocFormatter(MockCreateDatabase, "CREATE DATABASE %? TABLESPACE %?");
	AdHocFormatter(MockDropTablespace, "DROP TABLESPACE IF EXISTS %?");
	AdHocFormatter(MockDropTablespaceDir, "COPY (SELECT '%?') TO PROGRAM 'xargs rm -rf'");

	void
	Mock::CreateNewDatabase() const
	{
		if (hasCopyToProgram()) {
			DropDatabase();
			master->execute(MockCreateTablespaceDir::get(tablespacePath));
			master->execute(MockCreateTablespace::get(testDbName, tablespacePath.string()));
			master->execute(MockCreateDatabase::get(testDbName, testDbName));
		}
		else {
			MockServerDatabase::CreateNewDatabase();
		}
	}

	void
	Mock::DropDatabase() const
	{
		auto t = master->modify(
				"SELECT pg_terminate_backend(pid) FROM pg_stat_activity WHERE LOWER(datname) = LOWER(?)");
		t->bindParamS(0, testDbName);
		t->execute();
		MockServerDatabase::DropDatabase();
		if (hasCopyToProgram()) {
			master->execute(MockDropTablespace::get(testDbName));
			master->execute(MockDropTablespaceDir::get(tablespacePath));
		}
	}
}
