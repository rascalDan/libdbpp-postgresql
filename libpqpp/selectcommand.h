#ifndef PQ_SELECTCOMMAND_H
#define PQ_SELECTCOMMAND_H

#include "../libdbpp/selectcommand.h"
#include "command.h"
#include <vector>
#include <map>

namespace PQ {
	class Connection;
	class Column;
	class SelectCommand : public DB::SelectCommand, public Command {
		public:
			SelectCommand(const Connection *, const std::string & sql, unsigned int no);
			virtual ~SelectCommand();

			bool fetch();
			void execute();
			const DB::Column& operator[](unsigned int) const;
			const DB::Column& operator[](const Glib::ustring&) const;
			unsigned int columnCount() const;
			unsigned int getOrdinal(const Glib::ustring&) const;
		private:
			mutable bool executed;
			mutable bool txOpened;
			std::vector<Column *> fields;
			std::map<Glib::ustring, Column *> fieldsName;
			int nTuples, tuple;
			PGresult * execRes;

			friend class Column;
	};
}

#endif


