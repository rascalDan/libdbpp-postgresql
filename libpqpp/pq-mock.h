#ifndef MOCKPQDATASOURCE_H
#define MOCKPQDATASOURCE_H

#include "pq-connection.h"
#include <filesystem>
#include <mockDatabase.h>
#include <visibility.h>

namespace PQ {
	class DLL_PUBLIC Mock : public DB::MockServerDatabase {
	public:
		Mock(const std::string & master, const std::string & name, const std::vector<std::filesystem::path> & ss);
		~Mock() override;

		SPECIAL_MEMBERS_MOVE_RO(Mock);

		[[nodiscard]] DB::ConnectionPtr openConnection() const override;

	protected:
		void CreateNewDatabase() const override;
		void DropDatabase() const override;
		void SetTablesToUnlogged() const;
		[[nodiscard]] bool hasUnloggedTables() const;
		[[nodiscard]] bool hasCopyToProgram() const;
		const std::filesystem::path tablespacePath;
		const int serverVersion;
	};
}

#endif
