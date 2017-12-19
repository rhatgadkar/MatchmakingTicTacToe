#include "child_server.h"
#include "write_named_pipe.h"
#include "utilities.h"
#include <string>
using namespace std;

void ChildServer::run()
{
	// accept client 1

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
