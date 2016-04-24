#ifndef PQ_BULKSELECTCOMMAND_H
#define PQ_BULKSELECTCOMMAND_H

#include <selectcommand.h>
#include "pq-selectbase.h"
#include "pq-command.h"
#include <vector>
#include <map>

namespace PQ {
	class Connection;
	class Column;
	class BulkSelectCommand : public DB::SelectCommand, public SelectBase, public Command {
		public:
			BulkSelectCommand(Connection *, const std::string & sql, unsigned int no);
			virtual ~BulkSelectCommand();

			bool fetch() override;
			void execute() override;

		private:
			mutable bool executed;
			std::string preparedSql;
	};
}

#endif


