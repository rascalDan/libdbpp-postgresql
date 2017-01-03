#ifndef PQ_SELECTBASE_H
#define PQ_SELECTBASE_H

#include <libpq-fe.h>
#include <selectcommand.h>

namespace PQ {
	class SelectBase : public DB::SelectCommand {
		friend class Column;

		protected:
			SelectBase(const std::string & sql);
			~SelectBase();

			void createColumns(PGresult *);

			int nTuples, tuple;
			PGresult * execRes;
	};
}

#endif

