#pragma once

#include "pq-column.h"

namespace DB {
	class HandleField;
}

namespace PQ {
	class SelectBase;

	class BinaryColumn : public Column {
	public:
		BinaryColumn(const SelectBase *, unsigned int field);

		void apply(DB::HandleField &) const override;

	private:
		template<std::integral T> [[nodiscard]] inline T valueAs() const;
	};
}
