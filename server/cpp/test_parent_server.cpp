#include "parent_server.h"
#include "connection.h"
#include "constants.h"
#include <string>
#include <queue>
#include <unordered_map>
#include <unistd.h>
#include <cassert>
#include <iostream>
#include "utilities.h"
using namespace std;

void testIncrementTotalPop()
{
	Connection c(PARENT_PORT);
	ParentServer ps(c);
	const unordered_map<int, int>& childServerPop = ps.getChildServerPop();
	unordered_map<int, int>::const_iterator it;
	unordered_map<int, int> originalChildServerPop;
	int originalTotalPop;
	bool result;

	// verify that total pop and childServerPop remain unchanged using
	// invalid port
	originalChildServerPop = childServerPop;
	originalTotalPop = ps.getTotalPop();
	result = ps.incrementTotalPop(3000);
	assert(result == false);
	assert(ps.getTotalPop() == originalTotalPop);
	assert(childServerPop.size() == originalChildServerPop.size());
	for (it = childServerPop.begin(); it != childServerPop.end(); it++)
		assert(it->second == originalChildServerPop[it->first]);

	// verify that total pop is incremented and childServerPop is
	// incremented for a valid port
	originalChildServerPop = childServerPop;
	originalTotalPop = ps.getTotalPop();
	result = ps.incrementTotalPop(4951);
	assert(result == true);
	assert(originalTotalPop + 1 == ps.getTotalPop());
	assert(childServerPop.size() == originalChildServerPop.size());
	for (it = childServerPop.begin(); it != childServerPop.end(); it++)
	{
		if (it->first != 4951)
			assert(it->second == originalChildServerPop[it->first]);
		else
			assert(it->second ==
					originalChildServerPop[it->first] + 1);
	}
}

void testFreeChildsThread()
{
	Connection c(PARENT_PORT);
	ParentServer ps(c);
	const unordered_map<int, int>& childServerPop = ps.getChildServerPop();
	const queue<int>& emptyServers = ps.getEmptyServers();
	unordered_map<int, int>::const_iterator it;
	unordered_map<int, int> originalChildServerPop;
	queue<int> originalEmptyServers;
	int originalTotalPop;
	string port;
	const NamedPipe& namedPipe = ps.getNamedPipe();

	// verify that total pop, childServerPop, and emptyServers remain
	// unchanged using invalid port
	originalChildServerPop = childServerPop;
	originalTotalPop = ps.getTotalPop();
	originalEmptyServers = emptyServers;
	port = "3000";
	namedPipe.writePipe(port, port.length());
	ps.startFreeChildsThread();
//	sleep(15);  // the thread should complete in 15 seconds
	assert(ps.getTotalPop() == originalTotalPop);
	assert(childServerPop.size() == originalChildServerPop.size());
	for (it = childServerPop.begin(); it != childServerPop.end(); it++)
		assert(it->second == originalChildServerPop[it->first]);
	assert(emptyServers.size() == originalEmptyServers.size());

	// verify that total pop is decremented, childServerPop for port is 0,
	// and emptyServers is incremented when using valid port
	int portInt;
	try
	{
		portInt = ps.getEmptyServerPort();
	}
	catch (...)
	{
		cerr << "empty servers queue is empty" << endl;
		return;
	}
	port = intToStr(portInt);
	ps.incrementTotalPop(portInt);
	assert(childServerPop.at(portInt) == 1);
	originalChildServerPop = childServerPop;
	originalTotalPop = ps.getTotalPop();
	originalEmptyServers = emptyServers;
	namedPipe.writePipe(port, port.length());
	ps.startFreeChildsThread();
//	sleep(15);  // the thread should complete in 15 seconds
	assert(ps.getTotalPop() == originalTotalPop - 1);
	assert(childServerPop.size() == originalChildServerPop.size());
	for (it = childServerPop.begin(); it != childServerPop.end(); it++)
	{
		if (it->first != portInt)
			assert(it->second == originalChildServerPop[it->first]);
		else
			assert(it->second == 0);
	}
	assert(emptyServers.size() == originalEmptyServers.size() + 1);
}

int main()
{
	cout << "Running test for incrementTotalPop..." << endl;
	testIncrementTotalPop();
	cout << "incrementTotalPop test passed!" << endl;
	cout << endl;

	cout << "Running test for freeChildsThread..." << endl;
	testFreeChildsThread();
	cout << "freeChildsThread test passed!" << endl;
	cout << endl;

	cout << "All tests passed!" << endl;

	return 0;
}
