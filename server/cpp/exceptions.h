#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <stdexcept>

class ServerException : public exception
{
public:
	virtual const char * what() const throw
	{
		return "ServerException occurred";
	}
} ConnectionError;

class TimeoutException : public exception
{
public:
	virtual const char * what() const throw
	{
		return "TimeoutException occurred";
	}
} TimeoutError;

class DisconnectException : public exception
{
public:
	virtual const char * what() const throw
	{
		return "DisconnectException occurred";
	}
} DisconnectError;

runtime_error RuntimeError;

#endif  // EXCEPTIONS_H
