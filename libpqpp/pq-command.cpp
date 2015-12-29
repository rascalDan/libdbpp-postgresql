#include "pq-command.h"
#include "pq-connection.h"
#include <stdlib.h>
#include <string.h>
#include <buffer.h>
#include <boost/date_time/posix_time/posix_time.hpp>

PQ::Command::Command(Connection * conn, const std::string & sql, unsigned int no) :
	DB::Command(sql),
	stmntName(stringbf("pStatement_%u_%p", no, this)),
	c(conn)
{
}

PQ::Command::~Command()
{
	for (std::vector<char *>::const_iterator i = values.begin(); i != values.end(); ++i) {
		free(*i);
	}
}

void
PQ::Command::prepareSql(std::string & psql, const std::string & sql)
{
	char buf[4];
	int p = 1;
	bool inquote = false;
	for(std::string::const_iterator i = sql.begin(); i != sql.end(); ++i) {
		if (*i == '?' && !inquote) {
			snprintf(buf, 4, "$%d", p++);
			psql += buf;
		}
		else if (*i == '\'') {
			inquote = !inquote;
			psql += *i;
		}
		else {
			psql += *i;
		}
	}
}

void
PQ::Command::paramsAtLeast(unsigned int n)
{
	if (values.size() <= n) {
		values.resize(n + 1, NULL);
		lengths.resize(n + 1, 0);
		formats.resize(n + 1, 0);
	}
	else {
		free(values[n]);
		values[n] = NULL;
	}
}

void
PQ::Command::bindParamI(unsigned int n, int v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], "%d", v);
	formats[n] = 0;
}
void
PQ::Command::bindParamI(unsigned int n, long int v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], "%ld", v);
	formats[n] = 0;
}
void
PQ::Command::bindParamI(unsigned int n, long long int v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], "%lld", v);
	formats[n] = 0;
}
void
PQ::Command::bindParamI(unsigned int n, unsigned int v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], "%u", v);
	formats[n] = 0;
}
void
PQ::Command::bindParamI(unsigned int n, long unsigned int v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], "%lu", v);
	formats[n] = 0;
}
void
PQ::Command::bindParamI(unsigned int n, long long unsigned int v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], "%llu", v);
	formats[n] = 0;
}
void
PQ::Command::bindParamB(unsigned int n, bool v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], "%s", v ? "true" : "false");
	formats[n] = 0;
}
void
PQ::Command::bindParamF(unsigned int n, double v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], "%g", v);
	formats[n] = 0;
}
void
PQ::Command::bindParamF(unsigned int n, float v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], "%g", v);
	formats[n] = 0;
}
void
PQ::Command::bindParamS(unsigned int n, const Glib::ustring & s)
{
	paramsAtLeast(n);
	values[n] = strndup(s.c_str(), s.bytes());
	formats[n] = 0;
	lengths[n] = s.bytes();
}
void
PQ::Command::bindParamT(unsigned int n, const boost::posix_time::time_duration & v)
{
	paramsAtLeast(n);
	auto buf = boost::posix_time::to_simple_string(v);
	values[n] = strdup(buf.c_str());
	formats[n] = 0;
	lengths[n] = buf.length();
}
void
PQ::Command::bindParamT(unsigned int n, const boost::posix_time::ptime & v)
{
	paramsAtLeast(n);
	auto buf = boost::posix_time::to_iso_extended_string(v);
	values[n] = strdup(buf.c_str());
	formats[n] = 0;
	lengths[n] = buf.length();
}
void
PQ::Command::bindNull(unsigned int n)
{
	paramsAtLeast(n);
}

