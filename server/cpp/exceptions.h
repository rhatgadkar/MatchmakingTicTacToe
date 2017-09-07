#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>

class ConnectionException : public exception
{
public:
	virtual const char * what() const throw
	{
		return "ConnectionException occurred";
	}
} ConnectionError;

#endif  // EXCEPTIONS_H
