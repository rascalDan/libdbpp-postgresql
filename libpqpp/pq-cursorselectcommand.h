#ifndef PQ_CURSORSELECTCOMMAND_H
#define PQ_CURSORSELECTCOMMAND_H

#include "command_fwd.h" // for CommandOptionsCPtr
#include "pq-command.h"
#include "pq-selectbase.h"
#include <string> // for string

namespace PQ {
	class Connection;

	class CursorSelectCommand : public SelectBase, public Command {
	public:
		CursorSelectCommand(
				Connection *, const std::string & sql, const PQ::CommandOptionsCPtr &, const DB::CommandOptionsCPtr &);
		~CursorSelectCommand() override;

		bool fetch() override;
		void execute() override;

	private:
		void fetchTuples();
		std::string mkdeclare() const;

		mutable bool executed;
		unsigned int fTuples;
		std::string s_declare;
		std::string s_fetch;
		std::string s_close;
	};
}

#endif
