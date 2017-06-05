#ifndef PG_BINARY_COLUMN_H
#define PG_BINARY_COLUMN_H

#include "pq-column.h"

namespace PQ {
	class BinaryColumn : public Column {
		public:
			BinaryColumn(const SelectBase *, unsigned int field);

			void apply(DB::HandleField &) const override;
	};
}

#endif


