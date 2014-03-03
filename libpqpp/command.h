#ifndef PQ_COMMAND_H
#define PQ_COMMAND_H

#include "../libdbpp/command.h"
#include <libpq-fe.h>
#include <vector>

namespace PQ {
	class Connection;
	class Command : public virtual DB::Command {
		public:
			Command(const Connection *, const std::string & sql, unsigned int no);
			virtual ~Command() = 0;

			void bindParamI(unsigned int, int);
			void bindParamI(unsigned int, long int);
			void bindParamI(unsigned int, long long int);
			void bindParamI(unsigned int, unsigned int);
			void bindParamI(unsigned int, long unsigned int);
			void bindParamI(unsigned int, long long unsigned int);

			void bindParamF(unsigned int, double);
			void bindParamF(unsigned int, float);
			
			void bindParamS(unsigned int, const Glib::ustring&);
			
			void bindParamT(unsigned int, const boost::posix_time::time_duration &);
			void bindParamT(unsigned int, const boost::posix_time::ptime &);
			
			void bindNull(unsigned int);
		protected:
			const std::string stmntName;
			const Connection * c;

			void paramsAtLeast(unsigned int);
			std::vector<char *> values;
			std::vector<int> lengths;
			std::vector<int> formats;
	};
}

#endif


