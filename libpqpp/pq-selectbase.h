#ifndef PQ_SELECTBASE_H
#define PQ_SELECTBASE_H

#include "pq-command.h"
#include <libpq-fe.h>
#include <selectcommand.h>
#include <string>

namespace PQ {
	class SelectBase : public DB::SelectCommand {
		friend class Column;

	protected:
		SelectBase(const std::string & sql, const PQ::CommandOptionsCPtr & pqco);
		~SelectBase();

		void createColumns(PGresult *);

		unsigned int nTuples, tuple;
		PGresult * execRes;
		bool binary;
	};
}

#endif
