#ifndef PARENT_SERVER_H
#define PARENT_SERVER_H

#include <pthread.h>
#include "../read_named_pipe.h"
#include "../write_named_pipe.h"
#include "../server.h"
#include "parent_connection.h"

class ParentServer : public Server
{
public:
	ParentServer(ParentConnection& c);
	virtual ~ParentServer() {}
	virtual void run();

private:
	// Given a portStr, set the value of the corresponding port's pop in
	// the database to 0, push the port to the empty servers named pipe.
	pthread_t m_freeChildsThread;
	static void* freeChildsThread(void* args);
	void freeChildsAction(const std::string& portStr);
	void startFreeChildsThread();

	// the empty child servers with population 0
	const ReadNamedPipe m_emptyServersReadNamedPipe;
	const WriteNamedPipe m_emptyServersWriteNamedPipe;

	// child server ports that have a population of 1
	const ReadNamedPipe m_waitingServersReadNamedPipe;
	const WriteNamedPipe m_waitingServersWriteNamedPipe;

	// mapping of ports of child servers and their population is in portpop DB

	const ReadNamedPipe m_freePortReadNamedPipe;

	// Send total pop and child server port to the incoming client. If
	// child servers are full, send a 'full' message to the client.
	int handleSynPort();

	// Create a child server, by forking a process, which listens on the
	// specified port.
	void createMatchServer(int port);

	ParentConnection& m_parentConnection;
};

#endif  // PARENT_SERVER_H
