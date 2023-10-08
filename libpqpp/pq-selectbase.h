#pragma once

#include "pq-command.h"
#include <libpq-fe.h>
#include <selectcommand.h>
#include <string>

namespace PQ {
	class SelectBase : public DB::SelectCommand {
		friend class Column;

	protected:
		SelectBase(const std::string & sql, const PQ::CommandOptionsCPtr & pqco);

		void createColumns();

		unsigned int nTuples, tuple;
		ResultPtr execRes;
		bool binary;
	};
}
