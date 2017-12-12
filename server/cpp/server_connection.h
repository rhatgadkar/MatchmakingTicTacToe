#ifndef SERVER_CONNECTION_H
#define SERVER_CONNECTION_H

#include "connection.h"
#include <string>

class ServerConnection : public Connection
{
public:
	ServerConnection(int hostPort);
	virtual ~ServerConnection();
	int getHostPort() const { return m_hostPort; }
	virtual int getClientPort() const { return m_clientPort; }
	virtual std::string getClientIP() const { return m_clientIP; }
	virtual std::string receiveFrom(int time);
	virtual void sendTo(std::string text);
	virtual void acceptClient(int time = 0);
	virtual void closeClient();

private:
	int m_sockfd;
	int m_clientSockfd;
	int m_hostPort;
	int m_clientPort;
	std::string m_clientIP;
	struct addrinfo* m_servinfo;
};

#endif  // SERVER_CONNECTION_H
