#ifndef PQ_MODIFYCOMMAND_H
#define PQ_MODIFYCOMMAND_H

#include <modifycommand.h>
#include "pq-prepared.h"
#include "pq-connection.h"

namespace PQ {
	class ModifyCommand : public DB::ModifyCommand, public PreparedStatement {
		public:
			ModifyCommand(Connection *, const std::string & sql, const DB::CommandOptions *);
			virtual ~ModifyCommand();

			unsigned int execute(bool) override;
	};
}

#endif



