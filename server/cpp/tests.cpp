#include "mock_parent_connection.h"
#include "mock_child_connection.h"
#include "parent_server.h"
#include "child_server.h"
#include "constants.h"
#include <string>
#include <iostream>
#include <vector>
#include <cassert>
#include "exceptions.h"
#include "write_named_pipe.h"
#include "utilities.h"
using namespace std;

void testParentServer()
{
	MockParentConnection c;
	ParentServer p(c);
	const vector<string>* sentMsgs;

	// test send 0 pop and 4951 port
	// current variable values:
	// 	waitingServers: []
	// 	emptyServers: [ 4951, 4952, ..., 5050 ]
	// 	pop: 0
	// new variable values:
	// 	waitingServers: [ 4951 ]
	// 	emptyServers: [ 4952, 4953, ..., 5050 ]
	// 	pop: 1
	p.run();
	sentMsgs = &c.getSentMsgs();
	assert(sentMsgs->size() == 2);
	assert(sentMsgs->at(0) == "0");
	assert(sentMsgs->at(1) == "4951");
	c.clearSentMsgs();

	// test send 1 pop and 4951 port
	// current variable values:
	// 	waitingServers: [ 4951 ]
	// 	emptyServers: [ 4952, 4953, ..., 5050 ]
	// 	pop: 1
	// new variable values:
	// 	waitingServers: []
	// 	emptyServers: [ 4952, 4953, ..., 5050 ]
	// 	pop: 2
	p.run();
	sentMsgs = &c.getSentMsgs();
	assert(sentMsgs->size() == 2);
	assert(sentMsgs->at(0) == "1");
	assert(sentMsgs->at(1) == "4951");
	c.clearSentMsgs();

	// test send 2 pop and 4952 port
	// current variable values:
	// 	waitingServers: []
	// 	emptyServers: [ 4952, 4953, ..., 5050 ]
	// 	pop: 2
	// new variable values:
	// 	waitingServers: [ 4952 ]
	// 	emptyServers: [ 4953, 4954, ..., 5050 ]
	// 	pop: 3
	p.run();
	sentMsgs = &c.getSentMsgs();
	assert(sentMsgs->size() == 2);
	assert(sentMsgs->at(0) == "2");
	assert(sentMsgs->at(1) == "4952");
	c.clearSentMsgs();

	// test clear 4951 port, send 1 pop and 4952 port
	// current variable values:
	// 	waitingServers: [ 4952 ]
	// 	emptyServers: [ 4953, 4954, ..., 5050 ]
	// 	pop: 3
	// after clear 4951 port variable values:
	// 	waitingServers: [ 4952 ]
	// 	emptyServers: [ 4953, 4954, ..., 5050, 4951 ]
	// 	pop: 1
	// new variable values:
	// 	waitingServers: []
	// 	emptyServers: [ 4953, 4954, ..., 5050, 4951 ]
	// 	pop: 2
	{
		WriteNamedPipe wnp(false);
		wnp.writePipe("4951", PORT_LEN);
	}
	p.run();
	sentMsgs = &c.getSentMsgs();
	assert(sentMsgs->size() == 2);
	assert(sentMsgs->at(0) == "1");
	assert(sentMsgs->at(1) == "4952");
	c.clearSentMsgs();

	// test send 2 pop and 4953 port
	// current variable values:
	// 	waitingServers: []
	// 	emptyServers: [ 4953, 4954, ..., 5050, 4951 ]
	// 	pop: 2
	// new variable values:
	// 	waitingServers: [ 4953 ]
	// 	emptyServers: [ 4954, 4955, ..., 5050, 4951 ]
	// 	pop: 3
	p.run();
	sentMsgs = &c.getSentMsgs();
	assert(sentMsgs->size() == 2);
	assert(sentMsgs->at(0) == "2");
	assert(sentMsgs->at(1) == "4953");
	c.clearSentMsgs();

	// test clear 4953 port, send 2 pop and 4954 port
	// current variable values:
	// 	waitingServers: [ 4953 ]
	// 	emptyServers: [ 4954, 4955, ..., 5050, 4951 ]
	// 	pop: 3
	// after clear 4953 port variable values:
	// 	waitingServers: []
	// 	emptyServers: [ 4954, 4955, ..., 5050, 4951, 4953 ]
	// 	pop: 2
	// new variable values:
	// 	waitingServers: [ 4954 ]
	// 	emptyServers: [ 4955, 4956, ..., 5050, 4951, 4953 ]
	// 	pop: 3
	{
		WriteNamedPipe wnp(false);
		wnp.writePipe("4953", PORT_LEN);
	}
	p.run();
	sentMsgs = &c.getSentMsgs();
	assert(sentMsgs->size() == 2);
	assert(sentMsgs->at(0) == "2");
	assert(sentMsgs->at(1) == "4954");
	c.clearSentMsgs();

	// test 200 pop
	// current variable values:
	// 	waitingServers: [ 4954 ]
	// 	emptyServers: [ 4955, 4956, ..., 5050, 4951, 4953 ]
	// 	pop: 3
	// new variable values:
	// 	waitingServers: []
	// 	emptyServers: []
	// 	pop: 200
	for (int pop = 3; pop < 200; pop++)
	{
		p.run();
		sentMsgs = &c.getSentMsgs();
		assert(sentMsgs->at(0) == intToStr(pop));
		c.clearSentMsgs();
	}

	// test 'full' gets sent after 200 pop
	// current variable values:
	// 	waitingServers: []
	// 	emptyServers: []
	// 	pop: 200
	// new variable values:
	// 	waitingServers: []
	// 	emptyServers: []
	// 	pop: 200
	p.run();
	sentMsgs = &c.getSentMsgs();
	assert(sentMsgs->at(0) == "200");
	assert(sentMsgs->at(1) == "full");
	c.clearSentMsgs();
}

void testChildServer()
{
	MockChildConnection c;
	ChildServer s(c, 4951);
	const vector<string>* client1SentMsgs;
	const vector<string>* client2SentMsgs;

	// test client 1, client 2 guest and client 1 win
	// client 1 send: "r,," "1" "3" "5" "w7"
	// client 2 send: "r,," "2" "4" "6"
	//	verify server send to client 1: "player-1" "r,," "2" "4" "6"
	//	verify server send to client 2: "r,," "1" "3" "5" "w7"
	list<string> client1Recv;
	client1Recv.push_back("r,,");
	client1Recv.push_back("1");
	client1Recv.push_back("3");
	client1Recv.push_back("5");
	client1Recv.push_back("w7");
	c.setClient1ReceivedMsgs(client1Recv);
	list<string> client2Recv;
	client1Recv.push_back("r,,");
	client1Recv.push_back("2");
	client1Recv.push_back("4");
	client1Recv.push_back("6");
	c.setClient2ReceivedMsgs(client2Recv);
	s.run();
	client1SentMsgs = &c.getClient1SentMsgs();
	client2SentMsgs = &c.getClient2SentMsgs();
	assert(client1SentMsgs->at(0) == "player-1");
	assert(client1SentMsgs->at(1) == "r,,");
	assert(client1SentMsgs->at(2) == "2");
	assert(client1SentMsgs->at(3) == "4");
	assert(client1SentMsgs->at(4) == "6");
	assert(client2SentMsgs->at(0) == "r,,");
	assert(client2SentMsgs->at(1) == "1");
	assert(client2SentMsgs->at(2) == "3");
	assert(client2SentMsgs->at(3) == "5");
	assert(client2SentMsgs->at(4) == "w7");
	c.clearClient1SentMsgs();
	c.clearClient2SentMsgs();
}

int main()
{
	testParentServer();
	cout << "All Parent Server tests passed!" << endl;

	testChildServer();
	cout << "All Child Server tests passed!" << endl;
}
