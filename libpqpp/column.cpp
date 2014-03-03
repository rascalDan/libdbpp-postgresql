#include "column.h"
#include "selectcommand.h"
#include "error.h"
#include <string.h>
#include <boost/date_time/posix_time/posix_time.hpp>

PQ::Column::Column(const SelectCommand * s, unsigned int i) :
	DB::Column(PQfname(s->execRes, i), i),
	sc(s),
	oid(PQftype(sc->execRes, colNo))
{
}

bool
PQ::Column::isNull() const
{
	return PQgetisnull(sc->execRes, sc->tuple, colNo);
}

void
PQ::Column::apply(DB::HandleField & h) const
{
	if (isNull()) {
		h.null();
		return;
	}
	switch (oid) {
		case 18: //CHAROID:
		case 1043: //VARCHAROID:
		case 25: //TEXTOID:
		case 142: //XMLOID:
			h.string(PQgetvalue(sc->execRes, sc->tuple, colNo), PQgetlength(sc->execRes, sc->tuple, colNo));
			break;
		case 16: //BOOLOID:
			h.integer(PQgetvalue(sc->execRes, sc->tuple, colNo)[0] == 't' ? 1 : 0);
			break;
		case 21: //INT2OID:
		case 23: //INT4OID:
		case 20: //INT8OID:
			h.integer(atol(PQgetvalue(sc->execRes, sc->tuple, colNo)));
			break;
		case 1700: //NUMERICOID:
		case 700: //FLOAT4OID:
		case 701: //FLOAT8OID:
			h.floatingpoint(atof(PQgetvalue(sc->execRes, sc->tuple, colNo)));
			break;
		case 704: //TINTERVALOID
		case 1083: //TIMEOID:
		case 1186: //INTERVALOID
			{
				int days = 0, hours = 0, minutes = 0, seconds = 0, fractions = 0, flen1 = 0, flen2 = 0;
				const char * val = PQgetvalue(sc->execRes, sc->tuple, colNo);
				if (sscanf(val, "%d days %d:%d:%d.%n%d%n", &days, &hours, &minutes, &seconds, &flen1, &fractions, &flen2) >= 4) {
					h.interval(boost::posix_time::time_duration((24 * days) + hours, minutes, seconds, fractions * pow(10, boost::posix_time::time_res_traits::num_fractional_digits() + flen1 - flen2)));
				}
				else if (sscanf(val, "%d day %d:%d:%d.%n%d%n", &days, &hours, &minutes, &seconds, &flen1, &fractions, &flen2) >= 4) {
					h.interval(boost::posix_time::time_duration((24 * days) + hours, minutes, seconds, fractions * pow(10, boost::posix_time::time_res_traits::num_fractional_digits() + flen1 - flen2)));
				}
				else {
					h.interval(boost::posix_time::duration_from_string(PQgetvalue(sc->execRes, sc->tuple, colNo)));
				}
				break;
			}
		case 702: //ABSTIMEOID:
		case 1082: //DATEOID:
		case 1114: //TIMESTAMPOID:
		case 1184: //TIMESTAMPTZOID:
			h.timestamp(boost::posix_time::time_from_string(PQgetvalue(sc->execRes, sc->tuple, colNo)));
			break;
		default:
			h.string(PQgetvalue(sc->execRes, sc->tuple, colNo), PQgetlength(sc->execRes, sc->tuple, colNo));
	}
}

void
PQ::Column::rebind(DB::Command *, unsigned int) const
{
	throw Error("Not supported");
}

