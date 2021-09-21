#include "pq-column.h"
#include "column.h"
#include "dbTypes.h"
#include "pq-selectbase.h"
#include <boost/date_time/gregorian/parsers.hpp>
#include <boost/date_time/posix_time/posix_time_config.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libpq-fe.h>
#include <memory>

PQ::Column::Column(const SelectBase * s, unsigned int i) :
	DB::Column(PQfname(s->execRes, static_cast<int>(i)), i), sc(s), oid(PQftype(sc->execRes, static_cast<int>(colNo))),
	buf(nullptr)
{
}

PQ::Column::~Column()
{
	if (buf) {
		PQfreemem(buf);
	}
}

bool
PQ::Column::isNull() const
{
	return PQgetisnull(sc->execRes, static_cast<int>(sc->tuple), static_cast<int>(colNo));
}

std::size_t
PQ::Column::length() const
{
	return static_cast<std::size_t>(PQgetlength(sc->execRes, static_cast<int>(sc->tuple), static_cast<int>(colNo)));
}

const char *
PQ::Column::value() const
{
	return PQgetvalue(sc->execRes, static_cast<int>(sc->tuple), static_cast<int>(colNo));
}

void
PQ::Column::apply(DB::HandleField & h) const
{
	if (isNull()) {
		h.null();
		return;
	}
	switch (oid) {
		case 18: // CHAROID:
		case 1043: // VARCHAROID:
		case 25: // TEXTOID:
		case 142: // XMLOID:
		default:
			h.string({value(), length()});
			break;
		case 16: // BOOLOID:
			h.boolean(value()[0] == 't');
			break;
		case 21: // INT2OID:
		case 23: // INT4OID:
		case 20: // INT8OID:
			h.integer(atol(value()));
			break;
		case 1700: // NUMERICOID:
		case 700: // FLOAT4OID:
		case 701: // FLOAT8OID:
			h.floatingpoint(atof(value()));
			break;
		case 704: // TINTERVALOID
		case 1083: // TIMEOID:
		case 1186: // INTERVALOID
		{
			int days = 0, hours = 0, minutes = 0, seconds = 0, fractions = 0, flen1 = 0, flen2 = 0;
			const char * val = value();
			// NOLINTNEXTLINE(hicpp-vararg)
			if (sscanf(val, "%d %*[days] %d:%d:%d.%n%d%n", &days, &hours, &minutes, &seconds, &flen1, &fractions,
						&flen2)
					>= 4) {
				h.interval(boost::posix_time::time_duration((24 * days) + hours, minutes, seconds,
						fractions
								* static_cast<long>(pow(10,
										boost::posix_time::time_res_traits::num_fractional_digits() + flen1 - flen2))));
			}
			else {
				h.interval(boost::posix_time::duration_from_string(val));
			}
			break;
		}
		case 1082: // DATEOID:
			h.timestamp(boost::posix_time::ptime(boost::gregorian::from_string(value())));
			break;
		case 702: // ABSTIMEOID:
		case 1114: // TIMESTAMPOID:
		case 1184: // TIMESTAMPTZOID:
			h.timestamp(boost::posix_time::time_from_string(value()));
			break;
		case 17: // BYTEAOID
		{
			if (buf) {
				PQfreemem(buf);
			}
			size_t len;
			buf = PQunescapeBytea(reinterpret_cast<const unsigned char *>(value()), &len);
			h.blob(DB::Blob(buf, len));
			break;
		}
	}
}
