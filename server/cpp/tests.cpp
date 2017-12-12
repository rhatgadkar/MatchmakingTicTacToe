#include "mock_connection.h"
#include "parent_server.h"
#include <string>
#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

void testParentServer()
{
	MockConnection c;
	ParentServer p(c);

	// test send 0 pop and 4951 port
	p.run();
	const vector<string>& sentMsgs = c.getSentMsgs();
	assert(sentMsgs.size() == 2);
	assert(sentMsgs[0] == "0");
	assert(sentMsgs[1] == "4951");
}

int main()
{
	testParentServer();
	cout << "All tests passed!" << endl;
}
