#pragma once

#include "error.h" // IWYU pragma: export
#include <exception.h>
#include <libpq-fe.h>
#include <string>

// IWYU pragma: no_forward_declare DB::Error

namespace PQ {
	class Error : public AdHoc::Exception<DB::Error> {
	public:
		explicit Error(const PGconn *);

		std::string message() const noexcept override;

	private:
		std::string msg;
	};
}
