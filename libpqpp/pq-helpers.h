#ifndef PQ_HELPERS_H
#define PQ_HELPERS_H

namespace PQ {
	template<auto func> struct pq_deleter {
		void
		operator()(auto p) const
		{
			func(p);
		}
	};
}

#endif
