#ifndef CHILD_SERVER_H
#define CHILD_SERVER_H

#include "server.h"
#include "connection.h"

class ChildServer : public Server
{
public:
	ChildServer(Connection& c, int port) : Server(c), m_port(port) {}
	virtual ~ChildServer();
	virtual void run();

private:
	int m_port;
};

#endif  // CHILD_SERVER_H
