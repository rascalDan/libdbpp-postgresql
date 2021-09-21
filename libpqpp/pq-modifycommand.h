#ifndef PQ_MODIFYCOMMAND_H
#define PQ_MODIFYCOMMAND_H

#include "command_fwd.h"
#include "pq-prepared.h"
#include <modifycommand.h>
#include <string>

namespace PQ {
	class Connection;
	class ModifyCommand : public DB::ModifyCommand, public PreparedStatement {
	public:
		ModifyCommand(Connection *, const std::string & sql, const DB::CommandOptionsCPtr &);

		unsigned int execute(bool) override;
	};
}

#endif
