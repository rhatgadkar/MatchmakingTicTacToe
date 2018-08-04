#include <unistd.h>  // for sleep
#include "../constants.h"
#include "../read_named_pipe.h"
#include "../write_named_pipe.h"
#include <string>
#include <sys/wait.h>  // for waitpid
#include "../db-accessor.h"
#include "../exceptions.h"
#include "../utilities.h"
#include <iostream>
using namespace std;

int main()
{
	cout << "Starting cron-free-childs process ..." << endl;

	// mapping of ports of child servers and their population is in portpop DB	
	const ReadNamedPipe freePortReadNamedPipe(FREE_PORT_FIFO_NAME, false);

	// the empty child servers with population 0
	const WriteNamedPipe emptyServersWriteNamedPipe(EMPTY_SERVERS_FIFO_NAME, false);

	do
	{
		sleep(THREAD_INTERVAL);

		do
		{
			string portStr;
			try
			{
				portStr = freePortReadNamedPipe.readPipe(PORT_LEN);
				int port = strToInt(portStr);
				cout << "clearing port: " << port << endl;
				set_port_pop(port, 0);
				try
				{
					emptyServersWriteNamedPipe.writePipe(portStr, PORT_LEN);
				}
				catch (...)
				{
					cerr << "Failed to write empty port FIFO" << endl;
				}
			}
			catch (exception& e)
			{
				e.what();
				string msg = "cron-free-childs freePortReadNamedPipe.readPipe";
				cerr << msg << endl;
			}
		} while ((waitpid(-1, NULL, WNOHANG)) > 0);

	} while (FOREVER);

	return 0;
}
