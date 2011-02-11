#ifndef PQ_CONNECTION_H
#define PQ_CONNECTION_H

#include "../libdbpp/connection.h"
#include <libpq-fe.h>

namespace PQ {
	class Connection : public DB::Connection {
		public:
			Connection(const std::string & info);
			~Connection();

			int beginTx() const;
			int commitTx() const;
			int rollbackTx() const;
			bool inTx() const;
			void ping() const;
			DB::BulkDeleteStyle bulkDeleteStyle() const;
			DB::BulkUpdateStyle bulkUpdateStyle() const;

			DB::SelectCommand * newSelectCommand(const std::string & sql) const;
			DB::ModifyCommand * newModifyCommand(const std::string & sql) const;

			void checkResult(PGresult * res, int expected, const char * doing) const;
			void checkResultFree(PGresult * res, int expected, const char * doing) const;

			PGconn * conn;

		private:
			static bool checkResultInt(PGresult * res, int expected);

			mutable unsigned int txDepth;
			mutable unsigned int pstmntNo;
	};
}

#endif

