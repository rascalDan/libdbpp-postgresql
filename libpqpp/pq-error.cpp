#include "pq-error.h"
#include <string.h>

PQ::Error::Error(const PGconn * conn) :
	msg(PQerrorMessage(conn))
{
}

std::string
PQ::Error::message() const throw()
{
	return msg;
}

