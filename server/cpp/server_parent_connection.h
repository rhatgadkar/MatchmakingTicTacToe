#ifndef SERVER_PARENT_CONNECTION_H
#define SERVER_PARENT_CONNECTION_H

#include "parent_connection.h"
#include <string>
#include <netdb.h>

class ServerParentConnection : public ParentConnection
{
public:
	ServerParentConnection(int hostPort);
	virtual ~ServerParentConnection();
	int getHostPort() const { return m_hostPort; }
	virtual int getClientPort() const { return m_clientPort; }
	virtual std::string getClientIP() const { return m_clientIP; }
	virtual std::string receiveFrom(int time);
	virtual void sendTo(std::string text);
	virtual void acceptClient(int time = 0);
	virtual void closeClient();

private:
	int m_sockfd;
	int m_hostPort;
	struct addrinfo* m_servinfo;
	int m_clientSockfd;
	int m_clientPort;
	std::string m_clientIP;
};

#endif  // SERVER_PARENT_CONNECTION_H
