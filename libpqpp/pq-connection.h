#ifndef PQ_CONNECTION_H
#define PQ_CONNECTION_H

#include <connection.h>
#include <libpq-fe.h>
#include <visibility.h>

namespace PQ {
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
			void execute(const std::string & sql) const override;
			DB::BulkDeleteStyle bulkDeleteStyle() const override;
			DB::BulkUpdateStyle bulkUpdateStyle() const override;

			DB::SelectCommand * newSelectCommand(const std::string & sql) const override;
			DB::ModifyCommand * newModifyCommand(const std::string & sql) const override;

			int64_t insertId() const override;

			void	beginBulkUpload(const char *, const char *) const override;
			void	endBulkUpload(const char *) const override;
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

