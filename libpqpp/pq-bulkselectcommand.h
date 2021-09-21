#ifndef PQ_BULKSELECTCOMMAND_H
#define PQ_BULKSELECTCOMMAND_H

#include "command_fwd.h"
#include "pq-command.h"
#include "pq-prepared.h"
#include "pq-selectbase.h"
#include <string>

namespace PQ {
	class Connection;
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
