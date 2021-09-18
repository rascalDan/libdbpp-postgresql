#include "pq-selectbase.h"
#include "pq-binarycolumn.h"
#include "pq-column.h"
#include "pq-command.h"

PQ::SelectBase::SelectBase(const std::string & sql, const PQ::CommandOptionsCPtr & pqco) :
	DB::Command(sql), DB::SelectCommand(sql), nTuples(0), tuple(0), execRes(nullptr),
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
	auto nFields = PQnfields(execRes);
	for (decltype(nFields) f = 0; f < nFields; f += 1) {
		if (binary) {
			insertColumn(std::make_unique<BinaryColumn>(this, f));
		}
		else {
			insertColumn(std::make_unique<Column>(this, f));
		}
	}
}
