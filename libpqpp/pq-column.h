#ifndef PG_COLUMN_H
#define PG_COLUMN_H

#include <column.h>
#include <libpq-fe.h>

namespace PQ {
	class SelectBase;
	class Column : public DB::Column {
		public:
			Column(const SelectBase *, unsigned int field);
			~Column();

			bool isNull() const override;
			void apply(DB::HandleField &) const override;

		protected:
			const SelectBase * sc;
			const Oid oid;
			// Buffer for PQunescapeBytea
			mutable unsigned char * buf;
	};
}

#endif

