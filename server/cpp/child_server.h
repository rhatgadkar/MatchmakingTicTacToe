#ifndef CHILD_SERVER_H
#define CHILD_SERVER_H

#include "server.h"
#include "child_connection.h"
#include <string>

class ChildServer : public Server
{
public:
	ChildServer(ChildConnection& c, int port)
		: m_childConnection(c), m_port(port) {}
	virtual ~ChildServer();
	virtual void run();

private:
	bool isLoginProvided(const std::string& login, std::string& username,
			std::string& password);
	bool m_client1LoginProvided;
	bool m_client2LoginProvided;
	std::string m_client1Username;
	std::string m_client2Username;

	void setClientWinLossMsg(std::string& clientWinLossMsg,
			const std::string& username, bool loginProvided);
	std::string m_client1WinLossMsg;
	std::string m_client2WinLossMsg;

	int m_port;
	ChildConnection& m_childConnection;
};

#endif  // CHILD_SERVER_H
