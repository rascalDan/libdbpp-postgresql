#ifndef MOCKPQDATASOURCE_H
#define MOCKPQDATASOURCE_H

#include <mockDatabase.h>
#include <boost/filesystem/path.hpp>
#include <visibility.h>

namespace PQ {
	class DLL_PUBLIC Mock : public DB::MockServerDatabase {
		public:
			Mock(const std::string & master, const std::string & name, const std::vector<boost::filesystem::path> & ss);
			~Mock();

			DB::Connection * openConnection() const override;

		protected:
			void DropDatabase() const override;
			void SetTablesToUnlogged() const;
	};
}

#endif

