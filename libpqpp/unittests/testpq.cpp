#define BOOST_TEST_MODULE TestPQ
#include <boost/test/unit_test.hpp>

#include <definedDirs.h>
#include <dbpp/modifycommand.h>
#include <dbpp/selectcommand.h>
#include <dbpp/column.h>
#include <pq-mock.h>
#include <testCore.h>
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <pq-error.h>
#include <pq-connection.h>

class StandardMockDatabase : public PQ::Mock {
	public:
		StandardMockDatabase() : PQ::Mock("user=postgres dbname=postgres", "pqmock", {
				rootDir / "pqschema.sql" })
		{
		}
};

BOOST_GLOBAL_FIXTURE( StandardMockDatabase );

BOOST_FIXTURE_TEST_SUITE( Core, DB::TestCore );

BOOST_AUTO_TEST_CASE( transactions )
{
	auto ro = DB::MockDatabase::openConnectionTo("pqmock");

	BOOST_REQUIRE_EQUAL(false, ro->inTx());
	ro->beginTx();
	BOOST_REQUIRE_EQUAL(true, ro->inTx());
	ro->rollbackTx();
	BOOST_REQUIRE_EQUAL(false, ro->inTx());

	ro->beginTx();
	BOOST_REQUIRE_EQUAL(true, ro->inTx());
	ro->commitTx();
	BOOST_REQUIRE_EQUAL(false, ro->inTx());

	delete ro;
}

BOOST_AUTO_TEST_CASE( bindAndSend )
{
	auto rw = DB::MockDatabase::openConnectionTo("pqmock");

	auto mod = rw->newModifyCommand("INSERT INTO test VALUES(?, ?, ?, ?, ?, ?)");
	mod->bindParamI(0, testInt);
	mod->bindParamF(1, testDouble);
	mod->bindParamS(2, testString);
	mod->bindParamB(3, testBool);
	mod->bindParamT(4, testDateTime);
	mod->bindParamT(5, testInterval);
	mod->execute();
	delete mod;
	rw->commitTx();
	delete rw;
}

BOOST_AUTO_TEST_CASE( bindAndSelect )
{
	auto ro = DB::MockDatabase::openConnectionTo("pqmock");

	auto select = ro->newSelectCommand("SELECT * FROM test WHERE id = ?");
	select->bindParamI(0, testInt);
	select->execute();
	int rows = 0;
	while (select->fetch()) {
		assertColumnValueHelper(*select, 0, testInt);
		assertColumnValueHelper(*select, 1, testDouble);
		assertColumnValueHelper(*select, 2, testString);
		assertColumnValueHelper(*select, 3, testBool);
		assertColumnValueHelper(*select, 4, testDateTime);
		assertColumnValueHelper(*select, 5, testInterval);
		rows += 1;
	}
	delete select;
	BOOST_REQUIRE_EQUAL(1, rows);
	delete ro;
}

BOOST_AUTO_TEST_CASE( bindAndSelectOther )
{
	auto ro = DB::MockDatabase::openConnectionTo("pqmock");

	auto select = ro->newSelectCommand("SELECT * FROM test WHERE id != ?");
	select->bindParamI(0, testInt);
	select->execute();
	int rows = 0;
	while (select->fetch()) {
		assertColumnValueHelper(*select, 0, 4);
		assertColumnValueHelper(*select, 1, 123.45);
		assertColumnValueHelper(*select, 2, std::string("some text with a ; in it and a ' too"));
		assertColumnValueHelper(*select, 3, true);
		assertColumnValueHelper(*select, 4, boost::posix_time::ptime_from_tm({ 3, 6, 23, 27, 3, 115, 0, 0, 0, 0, 0}));
		assertColumnValueHelper(*select, 5, boost::posix_time::time_duration(38, 13, 12));
		rows += 1;
	}
	delete select;
	BOOST_REQUIRE_EQUAL(1, rows);
	delete ro;
}

BOOST_AUTO_TEST_CASE( testP2MockScriptDir )
{
	auto ro = DB::MockDatabase::openConnectionTo("pqmock");

	auto select = ro->newSelectCommand("SELECT path FROM test2");
	select->execute();
	while (select->fetch()) {
		std::string path;
		(*select)[0] >> path;
		BOOST_REQUIRE(boost::filesystem::exists(path));
	}
	delete select;
	delete ro;
}

BOOST_AUTO_TEST_CASE( bulkload )
{
	auto ro = DB::MockDatabase::openConnectionTo("pqmock");

	auto count = ro->newSelectCommand("SELECT COUNT(*) FROM bulktest");
	// Test empty
	ro->beginBulkUpload("bulktest", "");
	ro->endBulkUpload(NULL);
	assertScalarValueHelper(*count, 0);
	// Test sample file
	ro->beginBulkUpload("bulktest", "");
	std::ifstream in((rootDir / "bulk.sample").string());
	if (!in.good()) throw std::runtime_error("Couldn't open bulk.sample");
	char buf[BUFSIZ];
	for (std::streamsize r; (r = in.readsome(buf, sizeof(buf))) > 0; ) {
		ro->bulkUploadData(buf, r);
	}
	ro->endBulkUpload(NULL);
	assertScalarValueHelper(*count, 800);

	delete count;
	delete ro;
}

BOOST_AUTO_TEST_CASE( bigIterate )
{
	auto ro = DB::MockDatabase::openConnectionTo("pqmock");

	auto count = ro->newSelectCommand("SELECT * FROM bulktest");
	unsigned int rows = 0;
	while (count->fetch()) {
		rows += 1;
	}
	BOOST_REQUIRE_EQUAL(800, rows);

	delete count;
	delete ro;
}

BOOST_AUTO_TEST_CASE( insertId )
{
	auto ro = DB::MockDatabase::openConnectionTo("pqmock");
	auto ins = ro->newModifyCommand("INSERT INTO idtest(foo) VALUES(1)");
	for (int x = 1; x < 4; x++) {
		ins->execute();
		BOOST_REQUIRE_EQUAL(x, ro->insertId());
	}
	delete ins;
	delete ro;
}

BOOST_AUTO_TEST_CASE( reconnect )
{
	auto ro = DB::MockDatabase::openConnectionTo("pqmock");
	auto rok = DB::MockDatabase::openConnectionTo("pqmock");
	auto pqconn = dynamic_cast<PQ::Connection *>(ro);
	int pid1 = PQbackendPID(pqconn->conn);
	BOOST_REQUIRE(pid1);
	ro->ping();
	auto kil = rok->newModifyCommand("SELECT pg_terminate_backend(?)");
	kil->bindParamI(0, pid1);
	kil->execute();
	delete kil;
	ro->ping();
	int pid2 = PQbackendPID(pqconn->conn);
	BOOST_REQUIRE(pid2);
	BOOST_REQUIRE(pid1 != pid2);
	delete ro;
	delete rok;
}

BOOST_AUTO_TEST_CASE( reconnectInTx )
{
	auto ro = DB::MockDatabase::openConnectionTo("pqmock");
	auto rok = DB::MockDatabase::openConnectionTo("pqmock");
	auto pqconn = dynamic_cast<PQ::Connection *>(ro);
	int pid1 = PQbackendPID(pqconn->conn);
	BOOST_REQUIRE(pid1);
	ro->ping();
	ro->beginTx();
	auto kil = rok->newModifyCommand("SELECT pg_terminate_backend(?)");
	kil->bindParamI(0, pid1);
	kil->execute();
	delete kil;
	BOOST_REQUIRE_THROW(ro->ping(), PQ::ConnectionError);
	delete ro;
	delete rok;
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_CASE( connfail )
{
	BOOST_REQUIRE_THROW(DB::ConnectionFactory::createNew("postgresql", "host=localhost user=no"), PQ::ConnectionError);
}

BOOST_AUTO_TEST_CASE( ssl )
{
	auto conn = DB::ConnectionFactory::createNew("postgresql", "host=randomdan.homeip.net user=gentoo dbname=postgres sslmode=require");
	BOOST_REQUIRE(conn);
	auto pqconn = dynamic_cast<PQ::Connection *>(conn);
	BOOST_REQUIRE(pqconn);
	BOOST_REQUIRE(PQgetssl(pqconn->conn));
	delete conn;
}

