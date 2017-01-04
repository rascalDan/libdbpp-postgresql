#ifndef PQ_COMMAND_H
#define PQ_COMMAND_H

#include <command.h>
#include <libpq-fe.h>
#include <vector>

namespace PQ {
	class Connection;
	class Command : public virtual DB::Command {
		public:
			Command(Connection *, const std::string & sql, unsigned int no);
			virtual ~Command() = 0;

			void bindParamI(unsigned int, int) override;
			void bindParamI(unsigned int, long int) override;
			void bindParamI(unsigned int, long long int) override;
			void bindParamI(unsigned int, unsigned int) override;
			void bindParamI(unsigned int, long unsigned int) override;
			void bindParamI(unsigned int, long long unsigned int) override;

			void bindParamB(unsigned int, bool) override;

			void bindParamF(unsigned int, double) override;
			void bindParamF(unsigned int, float) override;

			void bindParamS(unsigned int, const Glib::ustring&) override;

			void bindParamT(unsigned int, const boost::posix_time::time_duration &) override;
			void bindParamT(unsigned int, const boost::posix_time::ptime &) override;

			void bindNull(unsigned int) override;
		protected:
			void prepareSql(std::stringstream & psql, const std::string & sql) const;
			const std::string stmntName;
			Connection * const c;

			void paramsAtLeast(unsigned int);
			template<typename ... T>
			void paramSet(unsigned int, const char * fmt, const T & ... t);
			void paramSet(unsigned int, const std::string &);
			std::vector<char *> values;
			std::vector<int> lengths;
			std::vector<std::string *> bufs;
	};
}

#endif

