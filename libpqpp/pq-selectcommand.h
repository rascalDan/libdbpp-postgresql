#ifndef PQ_SELECTCOMMAND_H
#define PQ_SELECTCOMMAND_H

#include <selectcommand.h>
#include "pq-command.h"
#include <vector>
#include <map>

namespace PQ {
	class Connection;
	class Column;
	class SelectCommand : public DB::SelectCommand, public Command {
		public:
			SelectCommand(Connection *, const std::string & sql, unsigned int no);
			virtual ~SelectCommand();

			bool fetch() override;
			void execute() override;

		private:
			void fetchTuples();
			std::string mkdeclare() const;
			std::string mkfetch() const;
			std::string mkclose() const;

			mutable bool executed;
			mutable bool txOpened;
			int nTuples, tuple, fTuples;
			PGresult * execRes;
			std::string s_declare;
			std::string s_fetch;
			std::string s_close;

			friend class Column;
	};
}

#endif


