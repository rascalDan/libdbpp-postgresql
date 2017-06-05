#include "pq-command.h"
#include "pq-connection.h"
#include <stdlib.h>
#include <string.h>
#include <compileTimeFormatter.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <factory.h>

NAMEDFACTORY("postgresql", PQ::CommandOptions, DB::CommandOptionsFactory);

AdHocFormatter(PQCommondStatement, "pStatement_id%?");
PQ::Command::Command(Connection * conn, const std::string & sql, const DB::CommandOptions * opts) :
	DB::Command(sql),
	hash(opts && opts->hash ? *opts->hash : std::hash<std::string>()(sql)),
	stmntName(PQCommondStatement::get(hash)),
	c(conn)
{
}

PQ::Command::~Command()
{
	for (auto i = values.size(); i-- > 0;) {
		if (bufs[i]) {
			delete bufs[i];
		}
		else if (formats[i] == 0) {
			free(values[i]);
		}
	}
}

PQ::CommandOptions::CommandOptions(std::size_t hash, const DB::CommandOptionsMap & map) :
	DB::CommandOptions(hash),
	fetchTuples(get(map, "page-size", 35)),
	useCursor(!isSet(map, "no-cursor")),
	fetchBinary(isSet(map, "fetch-binary"))
{
}

PQ::CommandOptions::CommandOptions(std::size_t hash,
		unsigned int ft,
		bool uc,
		bool fb) :
	DB::CommandOptions(hash),
	fetchTuples(ft),
	useCursor(uc),
	fetchBinary(fb)
{
}

AdHocFormatter(PQCommandParamName, "$%?");
void
PQ::Command::prepareSql(std::stringstream & psql, const std::string & sql) const
{
	if (values.empty()) {
		psql << sql;
		return;
	}
	int p = 1;
	bool inquote = false;
	for (const auto & i : sql) {
		if (i == '?' && !inquote) {
			PQCommandParamName::write(psql, p++);
		}
		else if (i == '\'') {
			inquote = !inquote;
			psql << i;
		}
		else {
			psql << i;
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
		bufs.resize(n + 1, NULL);
	}
	else {
		if (bufs[n]) {
			delete bufs[n];
			bufs[n] = nullptr;
		}
		else if (formats[n] == 0) {
			free(values[n]);
		}
		values[n] = NULL;
		formats[n] = 0;
	}
}

template<typename ... T>
void
PQ::Command::paramSet(unsigned int n, const char * fmt, const T & ... v)
{
	paramsAtLeast(n);
	lengths[n] = asprintf(&values[n], fmt, v...);
}

void
PQ::Command::paramSet(unsigned int n, const std::string & b)
{
	paramsAtLeast(n);
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
PQ::Command::bindParamBLOB(unsigned int n, const DB::Blob & v)
{
	paramsAtLeast(n);
	lengths[n] = v.len;
	formats[n] = 1;
	values[n] = reinterpret_cast<char *>(const_cast<void *>(v.data));
	delete bufs[n];
	bufs[n] = nullptr;
}
void
PQ::Command::bindNull(unsigned int n)
{
	paramsAtLeast(n);
}

