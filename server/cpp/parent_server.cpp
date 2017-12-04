#include "parent_server.h"
#include "utilities.h"
#include <pthread.h>
#include "constants.h"
#include "exceptions.h"
#include <unistd.h>  // for sleep
#include <sys/wait.h>  // for waitpid
#include <unordered_map>
#include <iostream>
using namespace std;

#define TEST

#ifndef TEST
#define FOREVER 1
#define THREAD_INTERVAL 10
#else
#define FOREVER 0
#define THREAD_INTERVAL 0
#endif

ParentServer::ParentServer(const Connection& c) : m_connection(c)
{
	for (int port = PARENT_PORT + 1;
			port < PARENT_PORT + 1 + MAX_CHILD_SERVERS; port++)
	{
		m_childServerPop.insert({ port, 0 });
		m_emptyServers.push(port);
	}

	pthread_mutex_init(&m_emptyServersMutex, NULL);
	pthread_mutex_init(&m_popMutex, NULL);
	m_totalPop = 0;
}

int ParentServer::getTotalPop() const
{
	lockPopMutex();
	int totalPop = m_totalPop;
	unlockPopMutex();
	return totalPop;
}

int ParentServer::getEmptyServerPort()
{
	lockEmptyServersMutex();
	if (m_emptyServers.empty())
	{
		unlockEmptyServersMutex();
		throw std::runtime_error("empty servers queue is empty");
	}
	int emptyServerPort = m_emptyServers.front();
	m_emptyServers.pop();
	unlockEmptyServersMutex();
	return emptyServerPort;

}

void ParentServer::serverAction()
{
}

bool ParentServer::incrementTotalPop(int port)
{
	/* Increments total pop and updates m_childServerPop if the result is
	 * <= 2.  Returns true in this case.  Otherwise return false.
	*/

	unordered_map<int, int>::iterator foundChild;
	lockPopMutex();
	foundChild = m_childServerPop.find(port);
	if (foundChild != m_childServerPop.end())
	{
		if (foundChild->second >= 2)
		{
			unlockPopMutex();
			return false;
		}
		foundChild->second++;
		m_totalPop++;
		unlockPopMutex();
		return true;
	}
	unlockPopMutex();
	return false;
}

void ParentServer::startFreeChildsThread()
{
	pthread_create(&m_freeChildsThread, NULL,
			&ParentServer::freeChildsThread, this);

	if (!FOREVER)
		pthread_join(m_freeChildsThread, NULL);
}

void ParentServer::freeChildsAction(const string& portStr)
{
	/* Given a portStr, set the value of the corresponding port in
	 * m_childServerPop to 0, push the port to the m_emptyServers queue,
	 * and reduce the m_totalPop.
	*/

	int port = strToInt(portStr);
	unordered_map<int, int>::iterator foundChild;
	lockPopMutex();
	foundChild = m_childServerPop.find(port);
	if (foundChild != m_childServerPop.end() && foundChild->second != 0)
	{
		cout << "clearing port: " << port << endl;
		m_totalPop -= foundChild->second;
		foundChild->second = 0;
		lockEmptyServersMutex();
		m_emptyServers.push(port);
		unlockEmptyServersMutex();
	}
	unlockPopMutex();
}

void* ParentServer::freeChildsThread(void* args)
{
	ParentServer* ps = (ParentServer*)args;

	do
	{
		sleep(THREAD_INTERVAL);

		do
		{
			string portStr;
			try
			{
				portStr = ps->m_namedPipe.readPipe(PORT_LEN);
			}
			catch (exception& e)
			{
				e.what();
				cerr <<
					"ParentServer::freeChildsThread::readPipe"
					<< endl;
			}
			ps->freeChildsAction(portStr);
		} while ((waitpid(-1, NULL, WNOHANG)) > 0);

	} while (FOREVER);

	return NULL;
}
