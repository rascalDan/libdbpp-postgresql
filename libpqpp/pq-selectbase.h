#ifndef PQ_SELECTBASE_H
#define PQ_SELECTBASE_H

#include <libpq-fe.h>

namespace PQ {
	class SelectBase {
		friend class Column;

		protected:
			SelectBase();
			~SelectBase() = default;

			int nTuples, tuple;
			PGresult * execRes;
	};
}

#endif

