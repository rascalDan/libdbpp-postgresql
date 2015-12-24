#include "pq-error.h"
#include <string.h>

PQ::Error::Error() :
	msg(NULL)
{
}

PQ::Error::Error(const PQ::Error & e) :
	msg(e.msg ? strdup(e.msg) : NULL)
{
}

PQ::Error::Error(const char * e) :
	msg(e ? strdup(e) : NULL)
{
}

PQ::Error::~Error() throw()
{
	free(msg);
}

const char *
PQ::Error::what() const throw()
{
	return msg ? msg : "No message";
}

PQ::ConnectionError::ConnectionError(const PGconn * conn) :
	PQ::Error(PQerrorMessage(conn))
{
}

