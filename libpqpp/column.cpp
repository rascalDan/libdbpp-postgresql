#include "column.h"
#include "selectcommand.h"
#include "error.h"
#include <string.h>

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
	struct tm tm;
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
		case 1083: //TIMEOID:
			memset(&tm, 0, sizeof(tm));
			strptime(PQgetvalue(sc->execRes, sc->tuple, colNo), "%T", &tm);
			h.timestamp(tm);
			break;
		case 1082: //DATEOID:
			memset(&tm, 0, sizeof(tm));
			strptime(PQgetvalue(sc->execRes, sc->tuple, colNo), "%F", &tm);
			h.timestamp(tm);
			break;
		case 702: //ABSTIMEOID:
		case 1114: //TIMESTAMPOID:
		case 1184: //TIMESTAMPTZOID:
			strptime(PQgetvalue(sc->execRes, sc->tuple, colNo), "%F %T", &tm);
			h.timestamp(tm);
			break;
		default:
			fprintf(stderr, "Unknown Oid %d\n", oid);
			throw Error("Unknown Oid");
	}
}

void
PQ::Column::rebind(DB::Command *, unsigned int) const
{
	throw Error("Not supported");
}

