#include "pq-selectbase.h"
#include "pq-column.h"

PQ::SelectBase::SelectBase(const std::string & sql) :
	DB::Command(sql),
	DB::SelectCommand(sql),
	nTuples(0),
	tuple(0),
	execRes(NULL)
{
}

PQ::SelectBase::~SelectBase()
{
	if (execRes) {
		PQclear(execRes);
	}
}

void
PQ::SelectBase::createColumns(PGresult * execRes)
{
	unsigned int nFields = PQnfields(execRes);
	for (unsigned int f = 0; f < nFields; f += 1) {
		insertColumn(DB::ColumnPtr(new Column(this, f)));
	}
}

