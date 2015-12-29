#ifndef PG_COLUMN_H
#define PG_COLUMN_H

#include <column.h>
#include <libpq-fe.h>

namespace PQ {
	class SelectCommand;
	class Column : public DB::Column {
		public:
			Column(const SelectCommand *, unsigned int field);

			bool isNull() const override;
			void apply(DB::HandleField &) const override;

		protected:
			const SelectCommand * sc;
			const Oid oid;
	};
}

#endif

