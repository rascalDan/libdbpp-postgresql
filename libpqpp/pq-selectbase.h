#ifndef PQ_SELECTBASE_H
#define PQ_SELECTBASE_H

#include <libpq-fe.h>
#include <selectcommand.h>
#include "pq-command.h"

namespace PQ {
	class SelectBase : public DB::SelectCommand {
		friend class Column;

		protected:
			SelectBase(const std::string & sql, const PQ::CommandOptionsCPtr & pqco);
			~SelectBase();

			void createColumns(PGresult *);

			int nTuples, tuple;
			PGresult * execRes;
			bool binary;
	};
}

#endif

