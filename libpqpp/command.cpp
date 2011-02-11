#include "command.h"
#include "connection.h"
#include <stdlib.h>
#include <string.h>

static std::string addrStr(void * p, unsigned int no) {
	std::string r;
	r.resize(30);
	r.resize(snprintf(const_cast<char *>(r.c_str()), 30, "pStatement-%u-%p", no, p));
	return r;
}

PQ::Command::Command(const Connection * conn, const std::string & sql, unsigned int no) :
	DB::Command(sql),
	stmntName(addrStr(this, no)),
	prepared(false),
	c(conn)
{
}

PQ::Command::~Command()
{
	for (std::vector<char *>::const_iterator i = values.begin(); i != values.end(); i++) {
		free(*i);
	}
}

void
PQ::Command::prepare() const
{
	if (!prepared) {
		std::string psql;
		psql.reserve(sql.length() + 20);
		char buf[4];
		int p = 1;
		for(std::string::const_iterator i = sql.begin(); i != sql.end(); i++) {
			if (*i == '?') {
				snprintf(buf, 4, "$%d", p++);
				psql += buf;
			}
			else {
				psql += *i;
			}
		}
		c->checkResultFree(PQprepare(
					c->conn, stmntName.c_str(), psql.c_str(), values.size(), NULL), PGRES_COMMAND_OK, __PRETTY_FUNCTION__);
		prepared = true;
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
PQ::Command::bindParamF(unsigned int n, double v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], "%f", v);
	formats[n] = 0;
}
void
PQ::Command::bindParamF(unsigned int n, float v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], "%f", v);
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
PQ::Command::bindParamT(unsigned int n, const tm * v)
{
	paramsAtLeast(n);
	values[n] = static_cast<char *>(malloc(20));
	formats[n] = 0;
	strftime(values[n], 20, "%F %T", v);
	lengths[n] = 19;
}
void
PQ::Command::bindParamT(unsigned int n, time_t v)
{
	struct tm t;
	gmtime_r(&v, &t);
	bindParamT(n, &t);
}
void
PQ::Command::bindNull(unsigned int n)
{
	paramsAtLeast(n);
}

