#ifndef MOCKPQDATASOURCE_H
#define MOCKPQDATASOURCE_H

#include <mockDatabase.h>
#include <filesystem>
#include <visibility.h>
#include "pq-connection.h"

namespace PQ {
	class DLL_PUBLIC Mock : public DB::MockServerDatabase {
		public:
			Mock(const std::string & master, const std::string & name, const std::vector<std::filesystem::path> & ss);
			~Mock();

			DB::ConnectionPtr openConnection() const override;

		protected:
			void CreateNewDatabase() const override;
			void DropDatabase() const override;
			void SetTablesToUnlogged() const;
			bool hasUnloggedTables() const;
			bool hasCopyToProgram() const;
			const std::filesystem::path tablespacePath;
			const int serverVersion;
	};
}

#endif

