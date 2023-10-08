#ifndef stuff
#define stuff

#include "command_fwd.h"
#include "pq-command.h"
#include <c++11Helpers.h>
#include <string>

namespace PQ {
	class Connection;

	class PreparedStatement : public Command {
	public:
		~PreparedStatement() override = default;
		SPECIAL_MEMBERS_DELETE(PreparedStatement);

	protected:
		PreparedStatement(Connection *, const std::string &, const DB::CommandOptionsCPtr &);

		const char * prepare() const;
		mutable const char * pstmt;
	};
}

#endif
