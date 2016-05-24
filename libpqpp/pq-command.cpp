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
	for (auto i = values.size(); i-- > 0;) {
		if (bufs[i]) {
			delete bufs[i];
		}
		else {
			free(values[i]);
		}
	}
}

void
PQ::Command::prepareSql(std::string & psql, const std::string & sql)
{
	char buf[10];
	int p = 1;
	bool inquote = false;
	for(std::string::const_iterator i = sql.begin(); i != sql.end(); ++i) {
		if (*i == '?' && !inquote) {
			psql.append(buf, snprintf(buf, 10, "$%d", p++));
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
		bufs.resize(n + 1, NULL);
	}
	else {
		if (bufs[n]) {
			delete bufs[n];
			bufs[n] = nullptr;
		}
		else {
			free(values[n]);
		}
		values[n] = NULL;
	}
}

template<typename ... T>
void
PQ::Command::paramSet(unsigned int n, const char * fmt, const T & ... v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], fmt, v...);
	delete bufs[n];
	bufs[n] = nullptr;
}

void
PQ::Command::paramSet(unsigned int n, const std::string & b)
{
	paramsAtLeast(n);
	delete bufs[n];
	bufs[n] = new std::string(b);
	lengths[n] = b.length();
	values[n] = const_cast<char *>(bufs[n]->data());
}

void
PQ::Command::bindParamI(unsigned int n, int v)
{
	paramSet(n, "%d", v);
}
void
PQ::Command::bindParamI(unsigned int n, long int v)
{
	paramSet(n, "%ld", v);
}
void
PQ::Command::bindParamI(unsigned int n, long long int v)
{
	paramSet(n, "%lld", v);
}
void
PQ::Command::bindParamI(unsigned int n, unsigned int v)
{
	paramSet(n, "%u", v);
}
void
PQ::Command::bindParamI(unsigned int n, long unsigned int v)
{
	paramSet(n, "%lu", v);
}
void
PQ::Command::bindParamI(unsigned int n, long long unsigned int v)
{
	paramSet(n, "%llu", v);
}
void
PQ::Command::bindParamB(unsigned int n, bool v)
{
	paramSet(n, "%s", v ? "true" : "false");
}
void
PQ::Command::bindParamF(unsigned int n, double v)
{
	paramSet(n, "%g", v);
}
void
PQ::Command::bindParamF(unsigned int n, float v)
{
	paramSet(n, "%g", v);
}
void
PQ::Command::bindParamS(unsigned int n, const Glib::ustring & s)
{
	paramSet(n, s);
}
void
PQ::Command::bindParamT(unsigned int n, const boost::posix_time::time_duration & v)
{
	paramSet(n, boost::posix_time::to_simple_string(v));
}
void
PQ::Command::bindParamT(unsigned int n, const boost::posix_time::ptime & v)
{
	paramSet(n, boost::posix_time::to_iso_extended_string(v));
}
void
PQ::Command::bindNull(unsigned int n)
{
	paramsAtLeast(n);
}

