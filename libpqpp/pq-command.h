#ifndef PQ_COMMAND_H
#define PQ_COMMAND_H

#include <command.h>
#include <libpq-fe.h>
#include <vector>
#include <visibility.h>
#include "pq-connection.h"

namespace PQ {
	class Connection;

	class DLL_PUBLIC CommandOptions : public DB::CommandOptions {
		public:
			CommandOptions(std::size_t, const DB::CommandOptionsMap &);
			CommandOptions(std::size_t hash,
					unsigned int fetchTuples = 35,
					bool useCursor = true,
					bool fetchBinary = false);

			unsigned int fetchTuples;
			bool useCursor;
			bool fetchBinary;
	};
	typedef std::shared_ptr<CommandOptions> CommandOptionsPtr;
	typedef std::shared_ptr<const CommandOptions> CommandOptionsCPtr;

	class Command : public virtual DB::Command {
		public:
			Command(Connection *, const std::string & sql, const DB::CommandOptionsCPtr &);
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
			void bindParamS(unsigned int, const std::string_view&) override;

			void bindParamT(unsigned int, const boost::posix_time::time_duration &) override;
			void bindParamT(unsigned int, const boost::posix_time::ptime &) override;

			void bindParamBLOB(unsigned int, const DB::Blob &) override;

			void bindNull(unsigned int) override;
		protected:
			void prepareSql(std::stringstream & psql, const std::string & sql) const;
			Connection::StatementHash hash;
			const std::string stmntName;
			Connection * const c;

			void paramsAtLeast(unsigned int);
			template<typename ... T>
			void paramSet(unsigned int, const char * fmt, const T & ... t);
			void paramSet(unsigned int, const std::string_view &);
			std::vector<char *> values;
			std::vector<int> lengths;
			std::vector<int> formats;
			std::vector<std::string *> bufs;
	};
}

#endif

