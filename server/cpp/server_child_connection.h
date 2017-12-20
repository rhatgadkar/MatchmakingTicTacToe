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
	virtual int getClientPort() const { return m_clientPort; }
	virtual std::string getClientIP() const { return m_clientIP; }
	virtual std::string receiveFrom(int time) { return ""; };
	virtual void sendTo(std::string text) {}
	virtual void acceptClient(int time = 0) {}
	virtual void closeClient() {}

private:
	int m_sockfd;
	int m_hostPort;
	struct addrinfo* m_servinfo;
	int m_clientSockfd;
	int m_clientPort;
	std::string m_clientIP;
};

#endif  // SERVER_CHILD_CONNECTION_H
