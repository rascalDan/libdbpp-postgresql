#include "pq-column.h"
#include "column.h"
#include "dbTypes.h"
#include "pq-selectbase.h"
#include <boost/date_time/gregorian_calendar.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libpq-fe.h>
#include <server/catalog/pg_type_d.h>

PQ::Column::Column(const SelectBase * s, unsigned int i) :
	DB::Column(PQfname(s->execRes.get(), static_cast<int>(i)), i), sc(s),
	oid(PQftype(sc->execRes.get(), static_cast<int>(colNo)))
{
}

bool
PQ::Column::isNull() const
{
	return PQgetisnull(sc->execRes.get(), static_cast<int>(sc->tuple), static_cast<int>(colNo));
}

std::size_t
PQ::Column::length() const
{
	return static_cast<std::size_t>(
			PQgetlength(sc->execRes.get(), static_cast<int>(sc->tuple), static_cast<int>(colNo)));
}

const char *
PQ::Column::value() const
{
	return PQgetvalue(sc->execRes.get(), static_cast<int>(sc->tuple), static_cast<int>(colNo));
}

void
PQ::Column::apply(DB::HandleField & h) const
{
	if (isNull()) {
		h.null();
		return;
	}
	switch (oid) {
		case CHAROID:
		case VARCHAROID:
		case TEXTOID:
		case XMLOID:
		default:
			h.string({value(), length()});
			break;
		case BOOLOID:
			h.boolean(value()[0] == 't');
			break;
		case INT2OID:
		case INT4OID:
		case INT8OID:
			h.integer(atol(value()));
			break;
		case NUMERICOID:
		case FLOAT4OID:
		case FLOAT8OID:
			h.floatingpoint(atof(value()));
			break;
		case TIMEOID:
		case INTERVALOID: {
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
		case DATEOID:
			h.timestamp(boost::posix_time::ptime(boost::gregorian::from_string(value())));
			break;
		case TIMESTAMPOID:
		case TIMESTAMPTZOID:
			h.timestamp(boost::posix_time::time_from_string(value()));
			break;
		case BYTEAOID: {
			size_t len = 0;
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			buf = BufPtr {PQunescapeBytea(reinterpret_cast<const unsigned char *>(value()), &len)};
			h.blob(DB::Blob(buf.get(), len));
			break;
		}
	}
}
