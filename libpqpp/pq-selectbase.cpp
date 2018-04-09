#include "pq-selectbase.h"
#include "pq-column.h"
#include "pq-binarycolumn.h"
#include "pq-command.h"

PQ::SelectBase::SelectBase(const std::string & sql, const PQ::CommandOptionsCPtr & pqco) :
	DB::Command(sql),
	DB::SelectCommand(sql),
	nTuples(0),
	tuple(0),
	execRes(NULL),
	binary(pqco ? pqco->fetchBinary : false)
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
		insertColumn(DB::ColumnPtr(binary ? new BinaryColumn(this, f) : new Column(this, f)));
	}
}

