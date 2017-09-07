#ifndef SERVER_H
#define SERVER_H

#include <string>

#define PARENT_PORT 4950
#define BACKLOG 20

class Server
{
public:
	Server(int hostPort);
	~Server();
	int getHostPort() const;
	int getClientPort() const;
	std::string getClientIP() const;
	virtual void acceptClient() = 0;

private:
	int m_sockfd;
	int m_hostPort;
	int m_clientPort;
	std::string m_clientIP;
	struct addrinfo *m_servinfo;
};

class ParentServer : public Server
{
};

class ChildServer : public Server
{
};

#endif  // SERVER_H
