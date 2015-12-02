#ifndef PQ_ERROR_H
#define PQ_ERROR_H

#include <error.h>
#include <libpq-fe.h>
#include <visibility.h>

namespace PQ {
	class DLL_PUBLIC Error : public DB::Error {
		public:
			Error();
			Error(const Error &);
			Error(const char *);
			~Error() throw();

			const char * what() const throw();

		private:
			char * msg;
	};
	class DLL_PUBLIC ConnectionError : public Error, public virtual DB::ConnectionError {
		public:
			ConnectionError(const PGconn *);
	};
}

#endif

