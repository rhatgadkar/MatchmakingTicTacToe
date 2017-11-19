#ifndef PARENT_SERVER_H
#define PARENT_SERVER_H

#include <pthread.h>
#include <queue>
#include <unordered_map>

class ParentServer : public Server
{
public:
	ParentServer();
	virtual void serverAction();
	void startFreeChildsThread()
	void freeChildsAction();  // TODO: unit test this

	// getters
	NamedPipe& getNamedPipe() const { return m_namedPipe; }
	std::queue<int>% getEmptyServers() const { return m_emptyServers; }
	std::unordered_map<int, int>& getChildServerPop() const
		{ return m_childServerPop; }
	int getTotalPop() const { return m_totalPop; }

	// setters
	void lockPopMutex() { pthread_mutex_lock(&m_popMutex); }
	void unlockPopMutex() { pthread_mutex_unlock(&m_popMutex); }
	void lockEmptyServersMutex()
		{ pthread_mutex_lock(&m_emptyServersMutex); }
	void unlockEmptyServersMutex()
		{ pthread_mutex_unlock(&m_EmptyServersMutex); }
	void setTotalPop(int pop) { m_totalPop = pop; }

private:
	pthread_t m_freeChildsThread;
	static void* freeChildsThread(void* args);

	std::queue<int> m_emptyServers;
	pthread_mutex_t m_emptyServersMutex;

	std::unordered_map<int, int> m_childServerPop;
	int m_totalPop;
	pthread_mutex_t m_popMutex;

	NamedPipe m_namedPipe;
};

#endif  // PARENT_SERVER_H
