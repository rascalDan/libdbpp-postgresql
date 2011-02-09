#include "error.h"
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
}

const char *
PQ::Error::what() const throw()
{
	return msg ? msg : "No message";
}

