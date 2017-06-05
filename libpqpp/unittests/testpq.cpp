#define BOOST_TEST_MODULE TestPQ
#include <boost/test/unit_test.hpp>

#include <definedDirs.h>
#include <modifycommand.h>
#include <selectcommand.h>
#include <column.h>
#include <pq-mock.h>
#include <testCore.h>
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <pq-error.h>
#include <pq-connection.h>
#include <pq-command.h>
#include <selectcommandUtil.impl.h>

class StandardMockDatabase : public PQ::Mock {
	public:
		StandardMockDatabase() : PQ::Mock("user=postgres dbname=postgres", "PQmock", {
				rootDir / "pqschema.sql" })
		{
		}
};

BOOST_GLOBAL_FIXTURE( StandardMockDatabase );

BOOST_FIXTURE_TEST_SUITE( Core, DB::TestCore );

BOOST_AUTO_TEST_CASE( transactions )
{
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");

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
	auto rw = DB::MockDatabase::openConnectionTo("PQmock");

	auto mod = rw->newModifyCommand("INSERT INTO test VALUES(?, ?, ?, ?, ?, ?)");
	mod->bindParamI(0, testInt);
	mod->bindParamF(1, testDouble);
	mod->bindParamS(2, testString);
	mod->bindParamB(3, testBool);
	mod->bindParamT(4, testDateTime);
	mod->bindParamT(5, testInterval);
	mod->execute();
	mod->bindParamI(0, (unsigned int)(testInt + 10));
	mod->bindParamF(1, (float)(testDouble + 10));
	mod->bindParamS(2, testString + " something");
	mod->bindParamB(3, true);
	mod->bindParamT(4, testDateTime);
	mod->bindParamT(5, testInterval);
	mod->execute();
	mod->bindNull(0);
	mod->bindParamI(1, (long long unsigned int)(testDouble + 10));
	mod->bindParamI(2, (long long int)testInt);
	mod->bindParamB(3, true);
	mod->bindNull(4);
	mod->bindNull(5);
	mod->execute();
	mod->bindNull(0);
	mod->bindParamI(1, (long unsigned int)(testDouble + 10));
	mod->bindParamI(2, (long int)testInt);
	mod->bindParamB(3, true);
	mod->bindParamS(4, "2016-01-01T12:34:56");
	mod->bindNull(5);
	mod->execute();
	delete mod;
	mod = rw->newModifyCommand("DELETE FROM test WHERE string = '?'");
	BOOST_REQUIRE_THROW(mod->execute(false), DB::NoRowsAffected);
	BOOST_REQUIRE_EQUAL(0, mod->execute(true));
	delete mod;
	delete rw;
}

BOOST_AUTO_TEST_CASE( bindAndSelect )
{
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");

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

BOOST_AUTO_TEST_CASE( selectInTx )
{
	auto db = DB::MockDatabase::openConnectionTo("PQmock");

	auto select = db->newSelectCommand("SELECT * FROM test");
	while (select->fetch()) { }
	delete select;
	db->finish();

	db->beginTx();
	select = db->newSelectCommand("SELECT * FROM test");
	while (select->fetch()) { }
	delete select;
	db->commitTx();
	db->finish();

	delete db;
}

BOOST_AUTO_TEST_CASE( bindAndSelectOther )
{
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");

	auto select = ro->newSelectCommand("SELECT * FROM test WHERE id != ? AND id != ?");
	select->bindParamI(0, testInt);
	select->bindParamI(1, testInt + 10);
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
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");

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
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");

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

BOOST_AUTO_TEST_CASE( nofetch )
{
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");
	auto count = ro->newSelectCommand("SELECT * FROM bulktest");
	count->execute();
	delete count;
	delete ro;
}

BOOST_AUTO_TEST_CASE( bigIterate )
{
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");

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
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");
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
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");
	auto rok = DB::MockDatabase::openConnectionTo("PQmock");
	auto pqconn = dynamic_cast<PQ::Connection *>(ro);
	int pid1 = PQbackendPID(pqconn->conn);
	BOOST_REQUIRE(pid1);
	ro->ping();
	ro->modify("TRUNCATE TABLE test")->execute();
	auto kil = rok->newModifyCommand("SELECT pg_terminate_backend(?)");
	kil->bindParamI(0, pid1);
	kil->execute();
	delete kil;
	usleep(5000);
	ro->ping();
	int pid2 = PQbackendPID(pqconn->conn);
	BOOST_REQUIRE(pid2);
	BOOST_REQUIRE(pid1 != pid2);
	ro->modify("TRUNCATE TABLE test")->execute();
	delete ro;
	delete rok;
}

BOOST_AUTO_TEST_CASE( reconnectInTx )
{
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");
	auto rok = DB::MockDatabase::openConnectionTo("PQmock");
	auto pqconn = dynamic_cast<PQ::Connection *>(ro);
	int pid1 = PQbackendPID(pqconn->conn);
	BOOST_REQUIRE(pid1);
	ro->ping();
	ro->beginTx();
	auto kil = rok->newModifyCommand("SELECT pg_terminate_backend(?)");
	kil->bindParamI(0, pid1);
	kil->execute();
	delete kil;
	usleep(5000);
	BOOST_REQUIRE_THROW(ro->ping(), DB::ConnectionError);
	delete ro;
	delete rok;
}

BOOST_AUTO_TEST_CASE( statementReuse )
{
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");
	auto pqconn = dynamic_cast<PQ::Connection *>(ro);
	BOOST_REQUIRE_EQUAL(pqconn->preparedStatements.size(), 0);
	ro->modify("DELETE FROM test")->execute();
	BOOST_REQUIRE_EQUAL(pqconn->preparedStatements.size(), 1);
	for (int y = 0; y < 4; y += 1) {
		auto m1 = ro->modify("INSERT INTO test(id) VALUES(?)");
		BOOST_REQUIRE_EQUAL(pqconn->preparedStatements.size(), y == 0 ? 1 : 2);
		for (int x = 0; x < 4; x += 1) {
			m1->bindParamI(0, x);
			m1->execute();
		}
	}
	BOOST_REQUIRE_EQUAL(pqconn->preparedStatements.size(), 2);
	auto select = ro->newSelectCommand("SELECT COUNT(id), SUM(id) FROM test");
	while (select->fetch()) {
		assertColumnValueHelper(*select, 0, 16);
		assertColumnValueHelper(*select, 1, 24);
	}
	delete select;
	delete ro;
}

BOOST_AUTO_TEST_CASE( bulkSelect )
{
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");
	PQ::CommandOptions co(0, 35, false);
	auto sel = ro->newSelectCommand("SELECT * FROM test WHERE id > ?", &co);
	sel->bindParamI(0, 1);
	int totalInt = 0, count = 0;
	sel->forEachRow<int64_t>([&totalInt, &count](auto i) {
			totalInt += i;
			count += 1;
		});
	delete sel;
	BOOST_REQUIRE_EQUAL(20, totalInt);
	BOOST_REQUIRE_EQUAL(8, count);
	delete ro;
}

BOOST_AUTO_TEST_CASE( selectWithSmallPages )
{
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");
	PQ::CommandOptions co(0, 1, true);
	auto sel = ro->newSelectCommand("SELECT * FROM test WHERE id > ?", &co);
	sel->bindParamI(0, 1);
	int totalInt = 0, count = 0;
	sel->forEachRow<int64_t>([&totalInt, &count](auto i) {
			totalInt += i;
			count += 1;
		});
	delete sel;
	BOOST_REQUIRE_EQUAL(20, totalInt);
	BOOST_REQUIRE_EQUAL(8, count);
	delete ro;
}

BOOST_AUTO_TEST_CASE( dateoid )
{
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");
	PQ::CommandOptions co(0, 1, false);
	auto sel = ro->select("SELECT '2017-01-08'::date", &co);
	for (const auto & r : sel->as<boost::posix_time::ptime>()) {
		BOOST_REQUIRE_EQUAL(boost::posix_time::ptime(boost::gregorian::date(2017, 1, 8)), r.value<0>());
	}
	delete ro;
}

BOOST_AUTO_TEST_CASE( insertReturning )
{
	auto ro = DB::MockDatabase::openConnectionTo("PQmock");
	PQ::CommandOptions co(0, 35, false);
	auto sel = ro->newSelectCommand("INSERT INTO test(id, fl) VALUES(1, 3) RETURNING id + fl", &co);
	int totalInt = 0, count = 0;
	sel->forEachRow<int64_t>([&totalInt, &count](auto i) {
			totalInt += i;
			count += 1;
		});
	delete sel;
	BOOST_REQUIRE_EQUAL(4, totalInt);
	BOOST_REQUIRE_EQUAL(1, count);
	delete ro;
}

BOOST_AUTO_TEST_CASE( closeOnError )
{
	auto ro = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("PQmock"));
	BOOST_REQUIRE_THROW({
			ro->select("SELECT * FROM test")->forEachRow<>([&ro](){
					ro->execute("nonsense");
				});
			}, DB::Error);
	BOOST_REQUIRE_THROW({
			ro->select("SELECT * FROM test")->forEachRow<>([&ro](){
				ro->select("SELECT * FROM test")->forEachRow<>([&ro](){
						ro->execute("nonsense");
					});
				});
			}, DB::Error);
	ro->beginTx();
	BOOST_REQUIRE_THROW({
			ro->select("SELECT * FROM test")->forEachRow<>([&ro](){
					ro->execute("nonsense");
				});
			}, DB::Error);
	BOOST_REQUIRE_THROW({
			ro->select("SELECT * FROM test")->forEachRow<>([&ro](){
				ro->select("SELECT * FROM test")->forEachRow<>([&ro](){
						ro->execute("nonsense");
					});
				});
			}, DB::Error);
	ro->commitTx();
}

BOOST_AUTO_TEST_CASE( blobs )
{
	auto ro = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("PQmock"));
	std::vector<char> buf(29);
	memcpy(&buf[0], "This is some binary text data", 29);
	auto ins = ro->modify("INSERT INTO blobtest(data) VALUES(?)");
	ins->bindParamBLOB(0, buf);
	ins->execute();

	ro->execute("UPDATE blobtest SET md5 = md5(data)");

	auto sel = ro->select("SELECT data, md5, length(data) FROM blobtest");
	for (const auto & r : sel->as<DB::Blob, std::string, int64_t>()) {
		// Assert the DB understood the insert
		BOOST_REQUIRE_EQUAL(r.value<2>(), buf.size());
		BOOST_REQUIRE_EQUAL(r.value<1>(), "37c7c3737f93e8d17e845deff8fa74d2");
		// Assert the fetch of the data is correct
		BOOST_REQUIRE_EQUAL(r.value<0>().len, buf.size());
		std::string str((const char *)r.value<0>().data, r.value<0>().len);
		BOOST_REQUIRE_EQUAL(str, "This is some binary text data");
		BOOST_REQUIRE(memcmp(r.value<0>().data, &buf[0], 29) == 0);
	}
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_CASE( connfail )
{
	BOOST_REQUIRE_THROW(DB::ConnectionFactory::createNew("postgresql", "host=localhost user=no"), DB::ConnectionError);
	try {
		DB::ConnectionFactory::createNew("postgresql", "host=localhost user=no");
	}
	catch (const DB::ConnectionError & e) {
		BOOST_REQUIRE(std::string(e.what()).find("\"no\""));
	}
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

