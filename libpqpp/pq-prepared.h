#ifndef stuff
#define stuff

#include "pq-command.h"
#include "pq-connection.h"

namespace PQ {
	class PreparedStatement : public Command {
		protected:
			PreparedStatement(Connection *, const std::string &, unsigned int, const DB::CommandOptions *);
			virtual ~PreparedStatement() = default;

			const char * prepare() const;
			Connection::StatementHash hash;
			mutable const char * pstmt;
	};
}

#endif

