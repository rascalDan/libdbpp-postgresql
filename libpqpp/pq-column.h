#ifndef PG_COLUMN_H
#define PG_COLUMN_H

#include "pq-helpers.h"
#include <column.h>
#include <cstring>
#include <libpq-fe.h>

namespace PQ {
	class SelectBase;

	class Column : public DB::Column {
	public:
		Column(const SelectBase *, unsigned int field);

		[[nodiscard]] bool isNull() const override;
		void apply(DB::HandleField &) const override;

	protected:
		const char * value() const;
		std::size_t length() const;

		const SelectBase * sc;
		const Oid oid;

		// Buffer for PQunescapeBytea
		struct Buffer {
			using BufPtr = std::unique_ptr<unsigned char, pq_deleter<PQfreemem>>;
			size_t row {};
			size_t length {};
			BufPtr data;
		};

		mutable Buffer buffer;
	};
}

#endif
