#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>

class Connection
{
public:
	Connection(int hostPort);
	virtual ~Connection();
	int getHostPort() const { return m_hostPort; }
	int getClientPort() const { return m_clientPort; }
	std::string getClientIP() const { return m_clientIP; }
	std::string receiveFrom(int time);
	void sendTo(std::string text);
	void acceptClient(int time = 0);
	void closeClient();

private:
	int m_sockfd;
	int m_clientSockfd;
	int m_hostPort;
	int m_clientPort;
	std::string m_clientIP;
	struct addrinfo* m_servinfo;
};

#endif  // CONNECTION_H
