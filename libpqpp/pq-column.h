#ifndef PG_COLUMN_H
#define PG_COLUMN_H

#include <column.h>
#include <cstring>
#include <libpq-fe.h>

namespace PQ {
	class SelectBase;
	class Column : public DB::Column {
	public:
		Column(const SelectBase *, unsigned int field);
		~Column() override;

		[[nodiscard]] bool isNull() const override;
		void apply(DB::HandleField &) const override;

	protected:
		template<typename T>
		inline T
		valueAs() const
		{
			T v {};
			std::memcpy(&v, value(), sizeof(T));
			return v;
		}
		const char * value() const;
		std::size_t length() const;

		const SelectBase * sc;
		const Oid oid;
		// Buffer for PQunescapeBytea
		mutable unsigned char * buf;
	};
}

#endif
