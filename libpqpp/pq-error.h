#ifndef PQ_ERROR_H
#define PQ_ERROR_H

#include <error.h>
#include <exception.h>
#include <libpq-fe.h>

namespace PQ {
	class Error : public AdHoc::Exception<DB::Error> {
	public:
		explicit Error(const PGconn *);

		std::string message() const noexcept override;

	private:
		std::string msg;
	};
}

#endif
