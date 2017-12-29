#ifndef CHILD_SERVER_H
#define CHILD_SERVER_H

#include "server.h"
#include "child_connection.h"
#include <string>
#include <pthread.h>

class ChildServer : public Server
{
public:
	ChildServer(ChildConnection& c, int port)
		: m_childConnection(c), m_port(port) {}
	virtual ~ChildServer();
	virtual void run();

private:
	class Client
	{
	public:
		void setLoginProvided(const std::string& login);

		void setWinLossMsg();

		// getters
		bool isLoginProvided() { return m_loginProvided; }
		std::string getUsername() { return m_username; }
		std::string getPassword() { return m_password; }
		std::string getWinLossMsg() { return m_winLossMsg; }

	private:
		bool m_loginProvided;
		std::string m_username;
		std::string m_password;
		std::string m_winLossMsg;
	} m_client1, m_client2;

	bool isClient2Connected();
	static void* receiveDisconnectClient1Thread(void* args);
	struct Client2ConnectedThreadArgs
	{
		bool* messageReceived;
		bool* client2AcceptExpired;
		ChildServer* childServer;
	};

	static void* client1MatchThread(void* args);
	static void* client2MatchThread(void* args);
	struct MatchThreadArgs
	{
		char* client1Record;
		char* client2Record;
		pthread_mutex_t* clientRecordMutex;
		ChildServer* childServer;
	};

	int m_port;
	ChildConnection& m_childConnection;
};

#endif  // CHILD_SERVER_H
