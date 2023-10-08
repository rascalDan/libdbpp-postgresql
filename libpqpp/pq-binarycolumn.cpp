#include "pq-binarycolumn.h"
#include "column.h"
#include "dbTypes.h"
#include "pq-column.h"
#include <cstdint>
#include <endian.h>
#include <error.h>
#include <server/catalog/pg_type_d.h>

PQ::BinaryColumn::BinaryColumn(const PQ::SelectBase * s, unsigned int f) : PQ::Column(s, f) { }

void
PQ::BinaryColumn::apply(DB::HandleField & h) const
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
			h.string({value(), length()});
			break;
		case BOOLOID:
			h.boolean(valueAs<bool>());
			break;
		case INT2OID:
			h.integer(static_cast<int64_t>(be16toh(valueAs<uint16_t>())));
			break;
		case INT4OID:
			h.integer(static_cast<int64_t>(be32toh(valueAs<uint32_t>())));
			break;
		case INT8OID:
			h.integer(static_cast<int64_t>(be64toh(valueAs<uint64_t>())));
			break;
		case BYTEAOID:
			h.blob(DB::Blob(value(), length()));
			break;
		default:
			throw DB::ColumnTypeNotSupported();
	}
}
