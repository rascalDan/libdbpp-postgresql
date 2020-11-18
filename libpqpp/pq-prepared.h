#ifndef stuff
#define stuff

#include "pq-command.h"

namespace PQ {
	class PreparedStatement : public Command {
	protected:
		PreparedStatement(Connection *, const std::string &, const DB::CommandOptionsCPtr &);
		virtual ~PreparedStatement() = default;

		const char * prepare() const;
		mutable const char * pstmt;
	};
}

#endif
