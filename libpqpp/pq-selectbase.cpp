#include "pq-selectbase.h"
#include "command.h"
#include "pq-binarycolumn.h"
#include "pq-column.h"
#include "pq-command.h"
#include <libpq-fe.h>
#include <memory>
#include <selectcommand.h>
#include <string>

PQ::SelectBase::SelectBase(const std::string & sql, const PQ::CommandOptionsCPtr & pqco) :
	DB::Command(sql), DB::SelectCommand(sql), nTuples(0), tuple(0), binary(pqco ? pqco->fetchBinary : false)
{
}

void
PQ::SelectBase::createColumns()
{
	auto nFields = PQnfields(execRes.get());
	for (decltype(nFields) f = 0; f < nFields; f += 1) {
		if (binary) {
			insertColumn(std::make_unique<BinaryColumn>(this, f));
		}
		else {
			insertColumn(std::make_unique<Column>(this, f));
		}
	}
}
