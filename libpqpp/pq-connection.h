#ifndef PQ_CONNECTION_H
#define PQ_CONNECTION_H

#include <connection.h>
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
			Connection(const std::string & info);
			~Connection();

			void finish() const override;
			int beginTx() const override;
			int commitTx() const override;
			int rollbackTx() const override;
			bool inTx() const override;
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

		private:
			static bool checkResultInt(PGresult * res, int expected, int alternative);

			mutable unsigned int txDepth;
			mutable unsigned int pstmntNo;
			mutable bool rolledback;
	};
}

#endif

