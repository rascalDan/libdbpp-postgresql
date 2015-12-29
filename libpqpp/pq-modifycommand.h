#ifndef PQ_MODIFYCOMMAND_H
#define PQ_MODIFYCOMMAND_H

#include <modifycommand.h>
#include "pq-command.h"

namespace PQ {
	class Connection;
	class ModifyCommand : public DB::ModifyCommand, public Command {
		public:
			ModifyCommand(const Connection *, const std::string & sql, unsigned int no);
			virtual ~ModifyCommand();

			unsigned int execute(bool) override;

		private:
			void prepare() const;
			mutable bool prepared;
	};
}

#endif



