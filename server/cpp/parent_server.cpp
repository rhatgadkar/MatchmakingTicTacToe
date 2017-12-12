#include "server.h"
#include "connection.h"
#include "server_connection.h"
#include "parent_server.h"
#include "child_server.h"
#include "utilities.h"
#include <pthread.h>
#include "constants.h"
#include "exceptions.h"
#include "read_named_pipe.h"
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

ParentServer::ParentServer(Connection& c) : Server(c)
{
	for (int port = PARENT_PORT + 1;
			port < PARENT_PORT + 1 + MAX_CHILD_SERVERS; port++)
	{
		m_childServerPop.insert({ port, 0 });
		m_emptyServers.push(port);
	}

	pthread_mutexattr_init(&m_emptyServersMutexAttr);
	pthread_mutexattr_settype(&m_emptyServersMutexAttr,
			PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_emptyServersMutex, &m_emptyServersMutexAttr);

	pthread_mutexattr_init(&m_popMutexAttr);
	pthread_mutexattr_settype(&m_popMutexAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_popMutex, &m_popMutexAttr);

	m_totalPop = 0;
}

int ParentServer::handleSynPort()
{
	try
	{
		m_connection.acceptClient();
	}
	catch (...)
	{
		throw runtime_error("Error in accepting client.");
	}
	cout << "Accepted client " << m_connection.getClientIP() << ":"
		<< m_connection.getClientPort() << "." << endl;

	lockPopMutex();
	int numPpl = m_totalPop;
	unlockPopMutex();
	string numPplStr = intToStr(numPpl);
	try
	{
		m_connection.sendTo(numPplStr);
	}
	catch (...)
	{
		throw runtime_error("Error in sending total pop to client.");
	}

	// find child server port
	bool foundPort = false;
	int port;
	lockEmptyServersMutex();
	while (!foundPort &&
			(!m_waitingServers.empty() || !m_emptyServers.empty()))
	{
		unlockEmptyServersMutex();
		// get first waiting server that has population 1
		while (!foundPort && !m_waitingServers.empty())
		{
			port = m_waitingServers.front();
			m_waitingServers.pop();
			lockPopMutex();
			if (m_childServerPop[port] == 1)
			{
				m_childServerPop[port]++;
				m_totalPop++;
				unlockPopMutex();
				foundPort = true;
			}
			else if (m_childServerPop[port] == 0)
			{
				unlockPopMutex();
				lockEmptyServersMutex();
				m_emptyServers.push(port);
				unlockEmptyServersMutex();
			}
			else
				unlockPopMutex();

		}
		lockEmptyServersMutex();
		// no waiting server found. get first empty server with
		// population 1
		while (!foundPort && !m_emptyServers.empty())
		{
			port = m_emptyServers.front();
			m_emptyServers.pop();
			unlockEmptyServersMutex();
			lockPopMutex();
			if (m_childServerPop[port] == 1)
			{
				unlockPopMutex();
				m_waitingServers.push(port);
				lockEmptyServersMutex();
				break;
			}
			else if (m_childServerPop[port] == 0)
			{
				m_childServerPop[port]++;
				m_totalPop++;
				unlockPopMutex();
				foundPort = true;
			}
			else
				unlockPopMutex();
			lockEmptyServersMutex();
		}
	}
	unlockEmptyServersMutex();
	if (!foundPort)
	{
		try
		{
			m_connection.sendTo("full");
		}
		catch (...)
		{
			string msg = "Error in sending 'full' to client.";
			throw runtime_error(msg);
		}
		throw runtime_error("No child servers available.");
	}

	string portStr = intToStr(port);
	try
	{
		m_connection.sendTo(portStr);
	}
	catch (...)
	{
		throw runtime_error("Error in sending port to client.");
	}

	cout << "Sent ACK to use port: " << port << endl;
	return port;
}

void ParentServer::createMatchServer(int port)
{
	pid_t child_pid;

	child_pid = fork();
	if (child_pid != 0)
	{
		// parent
	}
	else
	{
		// child
		ServerConnection childConnection(port);
		ChildServer childServer(childConnection, port);
		childServer.run();
		exit(0);
	}
}

void ParentServer::run()
{
	startFreeChildsThread();

	do
	{
		int port;
		try
		{
			port = handleSynPort();
		}
		catch (...)
		{
			m_connection.closeClient();
			continue;
		}
		m_connection.closeClient();

		lockPopMutex();
		if (m_childServerPop[port] == 1)
		{
			unlockPopMutex();
			if (!FOREVER)
				return;
			createMatchServer(port);
		}
		else
			unlockPopMutex();

	} while (FOREVER);
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
				portStr =
					ps->m_readNamedPipe.readPipe(PORT_LEN);
			}
			catch (exception& e)
			{
				e.what();
				string msg = "ParentServer::freeChildsThread";
				msg += "::readPipe";
				cerr << msg << endl;
			}
			ps->freeChildsAction(portStr);
		} while ((waitpid(-1, NULL, WNOHANG)) > 0);

	} while (FOREVER);

	return NULL;
}
