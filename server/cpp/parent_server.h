#ifndef PARENT_SERVER_H
#define PARENT_SERVER_H

#include <pthread.h>
#include <queue>
#include <unordered_map>
#include "read_named_pipe.h"
#include "server.h"
#include "parent_connection.h"

class ParentServer : public Server
{
public:
	ParentServer(ParentConnection& c);
	virtual ~ParentServer() {}
	virtual void run();

private:
	// Given a portStr, set the value of the corresponding port in
	// m_childServerPop to 0, push the port to the m_emptyServers queue,
	// and reduce the m_totalPop.
	pthread_t m_freeChildsThread;
	static void* freeChildsThread(void* args);
	void freeChildsAction(const std::string& portStr);
	void startFreeChildsThread();

	// the empty child servers with population 0
	std::queue<int> m_emptyServers;
	mutable pthread_mutex_t m_emptyServersMutex;
	pthread_mutexattr_t m_emptyServersMutexAttr;
	void lockEmptyServersMutex() const
	{
		pthread_mutex_lock(&m_emptyServersMutex);
	}
	void unlockEmptyServersMutex() const
	{
		pthread_mutex_unlock(&m_emptyServersMutex);
	}

	// child server ports that have a population of 1
	std::queue<int> m_waitingServers;

	// mapping of ports of child servers and their population
	std::unordered_map<int, int> m_childServerPop;
	int m_totalPop;
	mutable pthread_mutex_t m_popMutex;
	pthread_mutexattr_t m_popMutexAttr;
	void lockPopMutex() const
	{
		pthread_mutex_lock(&m_popMutex);
	}
	void unlockPopMutex() const
	{
		pthread_mutex_unlock(&m_popMutex);
	}

	const ReadNamedPipe m_readNamedPipe;

	// Send total pop and child server port to the incoming client. If
	// child servers are full, send a 'full' message to the client.
	int handleSynPort();

	// Create a child server, by forking a process, which listens on the
	// specified port.
	void createMatchServer(int port);

	ParentConnection& m_parentConnection;
};

#endif  // PARENT_SERVER_H
