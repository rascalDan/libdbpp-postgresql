#include "pq-prepared.h"
#include "pq-connection.h"

PQ::PreparedStatement::PreparedStatement(Connection * c, const std::string & sql, unsigned int no) :
	DB::Command(sql),
	Command(c, sql, no),
	pstmt(nullptr)
{
}

const char *
PQ::PreparedStatement::prepare() const
{
	if (pstmt) {
		return pstmt;
	}
	auto hash(std::hash<std::string>()(sql));
	auto i = c->preparedStatements.find(hash);
	if (i != c->preparedStatements.end()) {
		return (pstmt = i->second.c_str());
	}
	std::string psql;
	psql.reserve(sql.length() + 20);
	prepareSql(psql, sql);
	c->checkResultFree(PQprepare(
				c->conn, stmntName.c_str(), psql.c_str(), values.size(), NULL), PGRES_COMMAND_OK);
	return (pstmt = c->preparedStatements.insert({hash, stmntName}).first->second.c_str());
}

