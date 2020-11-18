#ifndef PQ_MODIFYCOMMAND_H
#define PQ_MODIFYCOMMAND_H

#include "pq-connection.h"
#include "pq-prepared.h"
#include <modifycommand.h>

namespace PQ {
	class ModifyCommand : public DB::ModifyCommand, public PreparedStatement {
	public:
		ModifyCommand(Connection *, const std::string & sql, const DB::CommandOptionsCPtr &);

		unsigned int execute(bool) override;
	};
}

#endif
