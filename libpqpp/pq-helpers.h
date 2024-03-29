#pragma once

#include "libpq-fe.h"
#include <memory>

namespace PQ {
	template<auto func> struct pq_deleter {
		void
		operator()(auto p) const
		{
			func(p);
		}
	};

	using ResultPtr = std::unique_ptr<PGresult, pq_deleter<PQclear>>;
}
