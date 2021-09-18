#include "pq-binarycolumn.h"
#include "pq-selectbase.h"
#include <error.h>

PQ::BinaryColumn::BinaryColumn(const PQ::SelectBase * s, unsigned int f) : PQ::Column(s, f) { }

void
PQ::BinaryColumn::apply(DB::HandleField & h) const
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
			h.string({value(), length()});
			break;
		case 16: // BOOLOID:
			h.boolean(valueAs<bool>());
			break;
		case 21: // INT2OID:
			h.integer(static_cast<int64_t>(be16toh(valueAs<uint16_t>())));
			break;
		case 23: // INT4OID:
			h.integer(static_cast<int64_t>(be32toh(valueAs<uint32_t>())));
			break;
		case 20: // INT8OID:
			h.integer(static_cast<int64_t>(be64toh(valueAs<uint64_t>())));
			break;
		case 17: // BYTEAOID
			h.blob(DB::Blob(value(), length()));
			break;
		default:
			throw DB::ColumnTypeNotSupported();
	}
}
