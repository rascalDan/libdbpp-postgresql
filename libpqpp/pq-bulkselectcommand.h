#ifndef PQ_BULKSELECTCOMMAND_H
#define PQ_BULKSELECTCOMMAND_H

#include "pq-prepared.h"
#include "pq-selectbase.h"
#include <map>
#include <vector>

namespace PQ {
	class Connection;
	class Column;
	class BulkSelectCommand : public SelectBase, public PreparedStatement {
	public:
		BulkSelectCommand(Connection *, const std::string & sql, const PQ::CommandOptionsCPtr & pqco,
				const DB::CommandOptionsCPtr &);

		bool fetch() override;
		void execute() override;

	private:
		mutable bool executed;
	};
}

#endif
