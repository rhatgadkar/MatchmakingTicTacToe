#ifndef SERVER_H
#define SERVER_H

#include "connection.h"

class Server
{
public:
	Server(Connection& c) : m_connection(c) {}
	virtual ~Server() {}
	virtual void run() = 0;

protected:
	Connection& m_connection;
};

#endif  // SERVER_H
