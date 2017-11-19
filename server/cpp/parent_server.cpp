#include "parent_server.h"
#include <pthread.h>
#include "constants.h"
#include "exceptions.h"
using namespace std;

ParentServer::ParentServer() : Server(PARENT_PORT)
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

void ParentServer::serverAction()
{
}

void ParentServer::startFreeChildsThread()
{
	pthread_create(m_freeChildsThread, NULL,
			&ParentServer::freeChildsThread, this);
}

void ParentServer::freeChildsAction(const string& portStr)
{

	int port = strToInt(portStr);
	unordered_map<int, int>::const_iterator foundChild;
	foundChild = ps->getChildServerPop.find(port);
	if (foundChild != ps->getChildServerPop.end())
	{
		cout << "clearing port: " << port << endl;
		ps->lockPopMutex();
		ps->setTotalPop(ps->getTotalPop() - foundChild->second);
		foundChild->second = 0;
		ps->lockEmptyServersMutex();
		ps->getEmptyServers.push(port);
		ps->unlockEmptyServersMutex();
		ps->unlockPopMutex();
	}
}

static void* ParentServer::freeChildsThread(void* args)
{
	ParentServer* ps = (ParentServer*)args;

	for(;;)
	{
		sleep(10);

		while ((waitpid(-1, NULL, WNOHANG)) > 0)
		{
			string portStr;
			try
			{
				portStr = ps->getNamedPipe().read(PORT_LEN);
			}
			catch (exception& e)
			{
				e.what();
				cerr << "ParentServer::freeChildsThread::read"
					<< endl;
			}
			ps->freeChildsAction(portStr);
		}
	}

	return NULL;
}
