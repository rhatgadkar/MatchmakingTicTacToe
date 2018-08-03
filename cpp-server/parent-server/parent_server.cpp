#include "parent_connection.h"
#include "parent_server.h"
#include "../utilities.h"
#include "../constants.h"
#include "../exceptions.h"
#include "../read_named_pipe.h"
#include "../write_named_pipe.h"
#include "../db-accessor.h"
#include <iostream>
#include <sys/wait.h>  // for waitpid
using namespace std;

ParentServer::ParentServer(ParentConnection& c) : m_parentConnection(c),
		m_emptyServersReadNamedPipe(EMPTY_SERVERS_FIFO_NAME),
		m_emptyServersWriteNamedPipe(EMPTY_SERVERS_FIFO_NAME, false),
		m_waitingServersReadNamedPipe(WAITING_SERVERS_FIFO_NAME),
		m_waitingServersWriteNamedPipe(WAITING_SERVERS_FIFO_NAME, false),
		m_freePortReadNamedPipe(FREE_PORT_FIFO_NAME)
{
	for (int port = PARENT_PORT + 1;
			port < PARENT_PORT + 1 + MAX_CHILD_SERVERS; port++)
	{
		string portStr = intToStr(port);
		set_port_pop(port, 0);
		try
		{
			m_emptyServersWriteNamedPipe.writePipe(portStr,
					PORT_LEN);
		}
		catch (...)
		{
			cerr << "Failed to write port to emptyServers pipe"
					<< endl;
		}
	}
}

int ParentServer::handleSynPort()
{
	try
	{
		m_parentConnection.acceptClient();
	}
	catch (...)
	{
		throw runtime_error("Error in accepting client.");
	}
	cout << "Accepted client " << m_parentConnection.getClientIP() << ":"
		<< m_parentConnection.getClientPort() << "." << endl;

	int numPpl = get_total_pop();
	string numPplStr = intToStr(numPpl);
	try
	{
		m_parentConnection.sendTo(numPplStr);
	}
	catch (...)
	{
		throw runtime_error("Error in sending total pop to client.");
	}

	// find child server port
	bool foundPort = false;
	int port;
	while (!foundPort)
	{
		bool foundWaitingPort = false;
		string waitingPort;
		bool foundEmptyPort = false;
		string emptyPort;

		// get first waiting server that has population 1
		try
		{
			waitingPort = m_waitingServersReadNamedPipe.readPipe(
					PORT_LEN);
			foundWaitingPort = true;
		}
		catch (...)
		{
			foundWaitingPort = false;
		}
		while (!foundPort && foundWaitingPort)
		{
			port = strToInt(waitingPort);
			int portPop = get_port_pop(port);
			if (portPop == 1)
			{
				portPop++;
				set_port_pop(port, portPop);
				foundPort = true;
				break;
			}
			else if (portPop == 0)
			{
				try
				{
					m_emptyServersWriteNamedPipe.writePipe(
							waitingPort, PORT_LEN);
					foundEmptyPort = true;
				}
				catch (...)
				{
					cerr << "Failed to write empty port FIFO" << endl;
				}
			}
			try
			{
				waitingPort = m_waitingServersReadNamedPipe.readPipe(
						PORT_LEN);
				foundWaitingPort = true;
			}
			catch (...)
			{
				foundWaitingPort = false;
			}

		}
		if (foundPort)
			break;

		// no waiting server found. get first empty server
		try
		{
			emptyPort = m_emptyServersReadNamedPipe.readPipe(PORT_LEN);
			foundEmptyPort = true;
		}
		catch (...)
		{
			foundEmptyPort = false;
		}
		while (!foundPort && foundEmptyPort)
		{
			port = strToInt(emptyPort);
			int portPop = get_port_pop(port);
			if (portPop == 1)
			{
				try
				{
					m_waitingServersWriteNamedPipe.writePipe(
							emptyPort, PORT_LEN);
					foundWaitingPort = true;
				}
				catch (...)
				{
					cerr << "Failed to write waiting port FIFO" << endl;
				}
				break;
			}
			else if (portPop == 0)
			{
				portPop++;
				set_port_pop(port, portPop);
				try
				{
					m_waitingServersWriteNamedPipe.writePipe(
							emptyPort, PORT_LEN);
					foundWaitingPort = true;
				}
				catch (...)
				{
					cerr << "Failed to write waiting port FIFO" << endl;
				}
				foundPort = true;
				break;
			}
			try
			{
				emptyPort = m_emptyServersReadNamedPipe.readPipe(
						PORT_LEN);
				foundEmptyPort = true;
			}
			catch (...)
			{
				foundEmptyPort = false;
			}
		}

		if (!foundWaitingPort && !foundEmptyPort)
			break;
	}
	if (!foundPort)
	{
		try
		{
			m_parentConnection.sendTo("full");
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
		m_parentConnection.sendTo(portStr);
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
		string portStr = intToStr(port);
		const char* argList[] = {
			"../child-server/a.out",
			portStr.c_str(),
			NULL
		};
		execv(argList[0], (char**) argList);
		exit(0);
	}
}

void ParentServer::run()
{
	startCronFreeChildsProcess();

	do
	{
		int port;
		try
		{
			port = handleSynPort();
		}
		catch (...)
		{
			m_parentConnection.closeClient();
			continue;
		}
		m_parentConnection.closeClient();

		int portPop = get_port_pop(port);
		if (portPop == 1)
		{
			if (!FOREVER)
				return;
			createMatchServer(port);
		}

	} while (FOREVER);
}

void ParentServer::startCronFreeChildsProcess()
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
		const char* argList[] = {
			"../cron-free-childs/a.out",
			NULL
		};
		execv(argList[0], (char**) argList);
		exit(0);
	}

	if (!FOREVER)
		waitpid(child_pid, NULL, 0);
}
