#ifndef PQ_CONNECTION_H
#define PQ_CONNECTION_H

#include <connection.h>
#include <set>
#include <libpq-fe.h>
#include <visibility.h>
#include "pq-error.h"

namespace PQ {
	class ConnectionError : public virtual Error, public virtual DB::ConnectionError {
		public:
			ConnectionError(const PGconn *);
	};

	class DLL_PUBLIC Connection : public DB::Connection {
		public:
			typedef std::size_t StatementHash;
			typedef std::map<StatementHash, std::string> PreparedStatements;

			Connection(const std::string & info);
			~Connection();

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

			void	beginBulkUpload(const char *, const char *) override;
			void	endBulkUpload(const char *) override;
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

