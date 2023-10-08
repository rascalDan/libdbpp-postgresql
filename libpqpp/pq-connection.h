#pragma once

#include "command_fwd.h"
#include "pq-error.h"
#include "pq-helpers.h"
#include <c++11Helpers.h>
#include <connection.h>
#include <cstddef>
#include <cstdint>
#include <libpq-fe.h>
#include <map>
#include <string>
#include <visibility.h>

namespace PQ {
	class ConnectionError : public virtual Error, public virtual DB::ConnectionError {
	public:
		explicit ConnectionError(const PGconn *);
	};

	class DLL_PUBLIC Connection : public DB::Connection {
	public:
		using StatementHash = std::size_t;
		using PreparedStatements = std::map<StatementHash, std::string>;

		explicit Connection(const std::string & info);
		~Connection() override;

		SPECIAL_MEMBERS_MOVE_RO(Connection);

		void beginTxInt() override;
		void commitTxInt() override;
		void rollbackTxInt() override;
		void ping() const override;
		void execute(const std::string & sql, const DB::CommandOptionsCPtr & = nullptr) override;
		DB::BulkDeleteStyle bulkDeleteStyle() const override;
		DB::BulkUpdateStyle bulkUpdateStyle() const override;

		DB::SelectCommandPtr select(const std::string & sql, const DB::CommandOptionsCPtr & = nullptr) override;
		DB::ModifyCommandPtr modify(const std::string & sql, const DB::CommandOptionsCPtr & = nullptr) override;

		int64_t insertId() override;
		int serverVersion() const;

		void beginBulkUpload(const char *, const char *) override;
		void endBulkUpload(const char *) override;
		size_t bulkUploadData(const char *, size_t) const override;

		template<std::same_as<ExecStatusType>... Expected>
		ResultPtr checkResult(PGresult * res, Expected... expected) const;

		PGconn * conn;
		mutable PreparedStatements preparedStatements;
	};
}
