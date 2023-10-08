#include "pq-command.h"
#include <compileTimeFormatter.h>
#include <dbTypes.h>
#include <factory.h>
#include <optional>
#include <utility>

namespace Glib {
	class ustring;
}

namespace PQ {
	class Connection;
}

NAMEDFACTORY("postgresql", PQ::CommandOptions, DB::CommandOptionsFactory)

AdHocFormatter(PQCommondStatement, "pStatement_id%?");

PQ::Command::Command(Connection * conn, const std::string & sql, const DB::CommandOptionsCPtr & opts) :
	DB::Command(sql), hash(opts && opts->hash ? *opts->hash : std::hash<std::string>()(sql)),
	stmntName(PQCommondStatement::get(hash)), c(conn)
{
}

PQ::CommandOptions::CommandOptions(std::size_t hash, const DB::CommandOptionsMap & map) :
	DB::CommandOptions(hash), fetchTuples(get(map, "page-size", 35U)), useCursor(!isSet(map, "no-cursor")),
	fetchBinary(isSet(map, "fetch-binary"))
{
}

PQ::CommandOptions::CommandOptions(std::size_t hash, unsigned int ft, bool uc, bool fb) :
	DB::CommandOptions(hash), fetchTuples(ft), useCursor(uc), fetchBinary(fb)
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
		values.resize(n + 1, nullptr);
		lengths.resize(n + 1, 0);
		formats.resize(n + 1, 0);
		bufs.resize(n + 1);
	}
}

AdHocFormatter(PQCommandParamFmt, "%?");

template<typename... T>
void
PQ::Command::paramSet(unsigned int n, T &&... v)
{
	paramsAtLeast(n);
	// cppcheck-suppress constStatement
	bufs[n] = std::make_unique<std::string>(PQCommandParamFmt::get(std::forward<T>(v)...));
	lengths[n] = static_cast<int>(bufs[n]->length());
	formats[n] = 0;
	values[n] = bufs[n]->data();
}

void
PQ::Command::paramSet(unsigned int n, const std::string_view b)
{
	paramsAtLeast(n);
	bufs[n] = std::make_unique<std::string>(b);
	lengths[n] = static_cast<int>(b.length());
	formats[n] = 0;
	values[n] = bufs[n]->data();
}

void
PQ::Command::bindParamI(unsigned int n, int v)
{
	paramSet(n, v);
}

void
PQ::Command::bindParamI(unsigned int n, long int v)
{
	paramSet(n, v);
}

void
PQ::Command::bindParamI(unsigned int n, long long int v)
{
	paramSet(n, v);
}

void
PQ::Command::bindParamI(unsigned int n, unsigned int v)
{
	paramSet(n, v);
}

void
PQ::Command::bindParamI(unsigned int n, long unsigned int v)
{
	paramSet(n, v);
}

void
PQ::Command::bindParamI(unsigned int n, long long unsigned int v)
{
	paramSet(n, v);
}

void
PQ::Command::bindParamB(unsigned int n, bool v)
{
	paramSet(n, v);
}

void
PQ::Command::bindParamF(unsigned int n, double v)
{
	paramSet(n, v);
}

void
PQ::Command::bindParamF(unsigned int n, float v)
{
	paramSet(n, v);
}

void
PQ::Command::bindParamS(unsigned int n, const Glib::ustring & s)
{
	paramSet(n, std::string_view(s.data(), s.bytes()));
}

void
PQ::Command::bindParamS(unsigned int n, const std::string_view s)
{
	paramSet(n, s);
}

void
PQ::Command::bindParamT(unsigned int n, const boost::posix_time::time_duration v)
{
	paramSet(n, boost::posix_time::to_simple_string(v));
}

void
PQ::Command::bindParamT(unsigned int n, const boost::posix_time::ptime v)
{
	paramSet(n, boost::posix_time::to_iso_extended_string(v));
}

void
PQ::Command::bindParamBLOB(unsigned int n, const DB::Blob & v)
{
	paramsAtLeast(n);
	lengths[n] = static_cast<int>(v.len);
	formats[n] = 1;
	values[n] = static_cast<const char *>(v.data);
	bufs[n].reset();
}

void
PQ::Command::bindNull(unsigned int n)
{
	paramsAtLeast(n);
	lengths[n] = 0;
	formats[n] = 0;
	values[n] = nullptr;
	bufs[n].reset();
}
