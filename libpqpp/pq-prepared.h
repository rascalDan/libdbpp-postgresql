#ifndef stuff
#define stuff

#include "command_fwd.h"
#include "pq-command.h"
#include <string>

namespace PQ {
	class Connection;

	class PreparedStatement : public Command {
	protected:
		PreparedStatement(Connection *, const std::string &, const DB::CommandOptionsCPtr &);
		virtual ~PreparedStatement() = default;

		const char * prepare() const;
		mutable const char * pstmt;
	};
}

#endif
