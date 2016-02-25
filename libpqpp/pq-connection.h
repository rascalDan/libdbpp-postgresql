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
			typedef std::hash<std::string>::result_type StatementHash;
			typedef std::map<StatementHash, std::string> PreparedStatements;

			Connection(const std::string & info);
			~Connection();

			void beginTxInt() override;
			void commitTxInt() override;
			void rollbackTxInt() override;
			void ping() const override;
			void execute(const std::string & sql) override;
			DB::BulkDeleteStyle bulkDeleteStyle() const override;
			DB::BulkUpdateStyle bulkUpdateStyle() const override;

			DB::SelectCommand * newSelectCommand(const std::string & sql) override;
			DB::ModifyCommand * newModifyCommand(const std::string & sql) override;

			int64_t insertId() override;

			void	beginBulkUpload(const char *, const char *) override;
			void	endBulkUpload(const char *) override;
			size_t bulkUploadData(const char *, size_t) const override;

			PGresult * checkResult(PGresult * res, int expected, int alternative = -1) const;
			void checkResultFree(PGresult * res, int expected, int alternative = -1) const;

			PGconn * conn;
			PreparedStatements preparedStatements;

		private:
			static bool checkResultInt(PGresult * res, int expected, int alternative);

			mutable unsigned int pstmntNo;
	};
}

#endif

