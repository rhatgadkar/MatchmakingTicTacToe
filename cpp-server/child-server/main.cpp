#include "child_server.h"
#include "server_child_connection.h"
#include "../utilities.h"
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char* argv[])
{
	// argv[0] contains the program name
	// argv[1] contains the port number
	if (argc < 2 || argc > 2)
	{
		cerr << "Must contain only 2 arguments." << endl;
		return -1;
	}
	string portStr(argv[1]);
	int port = strToInt(portStr);

	ServerChildConnection childConnection(port);
	ChildServer childServer(childConnection, port);
	childServer.run();

	return 0;
}
