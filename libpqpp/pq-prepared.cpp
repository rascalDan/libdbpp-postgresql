#include "pq-prepared.h"
#include "pq-connection.h"

PQ::PreparedStatement::PreparedStatement(Connection * c, const std::string & sql, const DB::CommandOptions * opts) :
	DB::Command(sql),
	Command(c, sql, opts),
	pstmt(nullptr)
{
}

const char *
PQ::PreparedStatement::prepare() const
{
	if (pstmt) {
		return pstmt;
	}
	auto i = c->preparedStatements.find(hash);
	if (i != c->preparedStatements.end()) {
		return (pstmt = i->second.c_str());
	}
	std::stringstream psql;
	prepareSql(psql, sql);
	c->checkResultFree(PQprepare(
				c->conn, stmntName.c_str(), psql.str().c_str(), values.size(), NULL), PGRES_COMMAND_OK);
	return (pstmt = c->preparedStatements.insert({hash, stmntName}).first->second.c_str());
}

