#ifndef PQ_SELECTCOMMAND_H
#define PQ_SELECTCOMMAND_H

#include "../libdbpp/selectcommand.h"
#include "command.h"
#include <vector>
#include <map>

namespace PQ {
	class Connection;
	class Column;
	class SelectCommand : public DB::SelectCommand, public Command {
		public:
			SelectCommand(const Connection *, const std::string & sql, unsigned int no);
			virtual ~SelectCommand();

			bool fetch();
			void execute();

		private:
			void fetchTuples();
			mutable bool executed;
			mutable bool txOpened;
			int nTuples, tuple;
			PGresult * execRes;

			friend class Column;
	};
}

#endif


