#ifndef PQ_ERROR_H
#define PQ_ERROR_H

#include <error.h>

namespace PQ {
	class Error : public DB::Error {
		public:
			Error();
			Error(const Error &);
			Error(const char *);
			~Error() throw();

			const char * what() const throw();

		private:
			char * msg;
	};
	class ConnectionError : public Error, public virtual DB::ConnectionError {
	};
}

#endif

