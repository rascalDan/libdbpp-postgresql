#ifndef PQ_CURSORSELECTCOMMAND_H
#define PQ_CURSORSELECTCOMMAND_H

#include "pq-selectbase.h"
#include "pq-command.h"
#include <vector>
#include <map>

namespace PQ {
	class Connection;
	class Column;
	class CursorSelectCommand : public SelectBase, public Command {
		public:
			CursorSelectCommand(Connection *, const std::string & sql, const PQ::CommandOptions *, const DB::CommandOptions *);
			virtual ~CursorSelectCommand();

			bool fetch() override;
			void execute() override;

		private:
			void fetchTuples();
			std::string mkdeclare() const;

			mutable bool executed;
			int fTuples;
			std::string s_declare;
			std::string s_fetch;
			std::string s_close;
	};
}

#endif


