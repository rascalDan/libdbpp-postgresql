#ifndef PQ_CONNECTION_H
#define PQ_CONNECTION_H

#include "pq-error.h"
#include <c++11Helpers.h>
#include <connection.h>
#include <libpq-fe.h>
#include <set>
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

		PGresult * checkResult(PGresult * res, int expected, int alternative = -1) const;
		void checkResultFree(PGresult * res, int expected, int alternative = -1) const;

		PGconn * conn;
		mutable PreparedStatements preparedStatements;

	private:
		static bool checkResultInt(PGresult * res, int expected, int alternative);
	};
}

#endif
