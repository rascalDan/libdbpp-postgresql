#pragma once

#include "command_fwd.h"
#include "pq-command.h"
#include "pq-selectbase.h"
#include <c++11Helpers.h>
#include <string>

namespace PQ {
	class Connection;

	class CursorSelectCommand : public SelectBase, public Command {
	public:
		CursorSelectCommand(
				Connection *, const std::string & sql, const PQ::CommandOptionsCPtr &, const DB::CommandOptionsCPtr &);
		~CursorSelectCommand() override;
		SPECIAL_MEMBERS_DELETE(CursorSelectCommand);

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
