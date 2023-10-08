#include "pq-binarycolumn.h"
#include "column.h"
#include "dbTypes.h"
#include "pq-column.h"
#include <bit>
#include <cstdint>
#include <endian.h>
#include <error.h>
#include <server/catalog/pg_type_d.h>

PQ::BinaryColumn::BinaryColumn(const PQ::SelectBase * s, unsigned int f) : PQ::Column(s, f) { }

template<std::integral T>
inline T
PQ::BinaryColumn::valueAs() const
{
	T v {};
	std::memcpy(&v, value(), sizeof(T));
	if constexpr (std::endian::native != std::endian::big && sizeof(T) > 1) {
		return std::byteswap(v);
	}
	return v;
}

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
			h.integer(valueAs<int16_t>());
			break;
		case INT4OID:
			h.integer(valueAs<int32_t>());
			break;
		case INT8OID:
			h.integer(valueAs<int64_t>());
			break;
		case BYTEAOID:
			h.blob(DB::Blob(value(), length()));
			break;
		default:
			throw DB::ColumnTypeNotSupported();
	}
}
