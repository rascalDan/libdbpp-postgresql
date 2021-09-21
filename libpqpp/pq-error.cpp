#include "pq-error.h"
#include <libpq-fe.h>

PQ::Error::Error(const PGconn * conn) : msg(PQerrorMessage(conn)) { }

std::string
PQ::Error::message() const noexcept
{
	return msg;
}
