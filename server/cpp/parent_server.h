#ifndef PARENT_SERVER_H
#define PARENT_SERVER_H

#include <pthread.h>
#include <queue>
#include <unordered_map>
#include "connection.h"
#include "named_pipe.h"

class ParentServer
{
public:
	ParentServer(const Connection& c);
	void serverAction();  // TODO: implement this
	void startFreeChildsThread();

	// setters
	bool incrementTotalPop(int port);

	// getters
	const std::unordered_map<int, int>& getChildServerPop() const
		{ return m_childServerPop; }
	const std::queue<int>& getEmptyServers() const
		{ return m_emptyServers; }
	int getTotalPop() const;
	const NamedPipe& getNamedPipe() const { return m_namedPipe; }
	int getEmptyServerPort();

private:
	pthread_t m_freeChildsThread;
	static void* freeChildsThread(void* args);
	void freeChildsAction(const std::string& portStr);

	std::queue<int> m_emptyServers;
	mutable pthread_mutex_t m_emptyServersMutex;
	void lockEmptyServersMutex() const
		{ pthread_mutex_lock(&m_emptyServersMutex); }
	void unlockEmptyServersMutex() const
		{ pthread_mutex_unlock(&m_emptyServersMutex); }

	std::unordered_map<int, int> m_childServerPop;
	int m_totalPop;
	mutable pthread_mutex_t m_popMutex;
	void lockPopMutex() const { pthread_mutex_lock(&m_popMutex); }
	void unlockPopMutex() const { pthread_mutex_unlock(&m_popMutex); }

	const NamedPipe m_namedPipe;

	const Connection& m_connection;
};

#endif  // PARENT_SERVER_H
