#include "child_server.h"
#include "write_named_pipe.h"
#include "utilities.h"
#include <string>
#include <iostream>
using namespace std;

void ChildServer::run()
{
	// accept client 1
	try
	{
		m_childConnection.acceptClient(15);
	}
	catch (...)
	{
		cout << "Closing child server." << endl;
		return;
	}

	// receive client 1 login

	// send player 1 ACK to client 1

	// if login provided, get win/loss records

	//
}

ChildServer::~ChildServer()
{
	WriteNamedPipe writeNamedPipe(false);
	string portStr = intToStr(m_port);
	writeNamedPipe.writePipe(portStr, portStr.length());
}
