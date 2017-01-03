#ifndef PQ_BULKSELECTCOMMAND_H
#define PQ_BULKSELECTCOMMAND_H

#include "pq-selectbase.h"
#include "pq-prepared.h"
#include <vector>
#include <map>

namespace PQ {
	class Connection;
	class Column;
	class BulkSelectCommand : public SelectBase, public PreparedStatement {
		public:
			BulkSelectCommand(Connection *, const std::string & sql, unsigned int no, const DB::CommandOptions *);

			bool fetch() override;
			void execute() override;

		private:
			mutable bool executed;
	};
}

#endif


