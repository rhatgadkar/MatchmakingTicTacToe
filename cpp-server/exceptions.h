#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <stdexcept>

class ConnectionException : public std::exception
{
public:
	virtual const char* what() const throw()
	{
		return "ServerException occurred";
	}
};
extern ConnectionException ConnectionError;

class TimeoutException : public std::exception
{
public:
	virtual const char* what() const throw()
	{
		return "TimeoutException occurred";
	}
};
extern TimeoutException TimeoutError;

class DisconnectException : public std::exception
{
public:
	virtual const char* what() const throw()
	{
		return "DisconnectException occurred";
	}
};
extern DisconnectException DisconnectError;

class IncorrectLoginException : public std::exception
{
public:
	virtual const char* what() const throw()
	{
		return "IncorrectLoginException occurred";
	}
};
extern IncorrectLoginException IncorrectLoginError;

class UserInGameException : public std::exception
{
public:
	virtual const char* what() const throw()
	{
		return "UserInGameException occurred";
	}
};
extern UserInGameException UserInGameError;

#endif  // EXCEPTIONS_H
