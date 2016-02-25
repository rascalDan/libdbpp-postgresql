#ifndef PQ_MODIFYCOMMAND_H
#define PQ_MODIFYCOMMAND_H

#include <modifycommand.h>
#include "pq-command.h"
#include "pq-connection.h"

namespace PQ {
	class ModifyCommand : public DB::ModifyCommand, public Command {
		public:
			ModifyCommand(Connection *, const std::string & sql, unsigned int no);
			virtual ~ModifyCommand();

			unsigned int execute(bool) override;

		private:
			Connection::PreparedStatements::const_iterator prepare() const;
			const Connection::StatementHash hash;
	};
}

#endif



