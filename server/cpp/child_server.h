#ifndef CHILD_SERVER_H
#define CHILD_SERVER_H

#include "server.h"
#include "child_connection.h"

class ChildServer : public Server
{
public:
	ChildServer(ChildConnection& c, int port)
		: m_childConnection(c), m_port(port) {}
	virtual ~ChildServer();
	virtual void run();

private:
	int m_port;
	ChildConnection& m_childConnection;
};

#endif  // CHILD_SERVER_H
