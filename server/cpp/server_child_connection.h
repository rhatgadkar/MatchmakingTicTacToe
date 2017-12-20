#ifndef SERVER_CHILD_CONNECTION_H
#define SERVER_CHILD_CONNECTION_H

#include "child_connection.h"
#include <string>
#include <netdb.h>

class ServerChildConnection : public ChildConnection
{
public:
	ServerChildConnection(int port) : m_hostPort(port) {}
	virtual ~ServerChildConnection() {}

	// client 1
	virtual int getClient1Port() const { return m_clientPort; }
	virtual std::string getClient1IP() const { return m_clientIP; }
	virtual std::string receiveFromClient1(int time) { return ""; };
	virtual void sendToClient1(std::string text) {}
	virtual void acceptClient1(int time = 0) {}
	virtual void closeClient1() {}

	// client 2
	virtual int getClient2Port() const { return m_clientPort; }
	virtual std::string getClient2IP() const { return m_clientIP; }
	virtual std::string receiveFromClient2(int time) { return ""; };
	virtual void sendToClient2(std::string text) {}
	virtual void acceptClient2(int time = 0) {}
	virtual void closeClient2() {}

private:
	int m_sockfd;
	int m_hostPort;
	struct addrinfo* m_servinfo;
	int m_clientSockfd;
	int m_clientPort;
	std::string m_clientIP;
};

#endif  // SERVER_CHILD_CONNECTION_H
