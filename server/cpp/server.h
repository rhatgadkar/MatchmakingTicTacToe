#ifndef SERVER_H
#define SERVER_H

#include <string>

class Server
{
public:
	Server(int hostPort);
	~Server();
	int getHostPort() const;
	int getClientPort() const;
	std::string getClientIP() const;
	virtual void serverAction() = 0;
	std::string receiveFrom(int time);
	void sendTo(std::string text);
	void acceptClient(int time = 0);

private:
	int m_sockfd;
	int m_hostPort;
	int m_clientPort;
	std::string m_clientIP;
	struct addrinfo *m_servinfo;
};

#endif  // SERVER_H
