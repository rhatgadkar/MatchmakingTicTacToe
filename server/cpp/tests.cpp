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
	const vector<string>* sendToClient1Msgs;
	const vector<string>* sendToClient2Msgs;
	vector<string> expectedSendToClient1Msgs;
	vector<string> expectedSendToClient2Msgs;

	// test client 1, client 2 guest and client 1 win
	// client 1 send: "," "1" "3" "5" "w7"
	// client 2 send: "," "2" "4" "6"
	//	verify server send to client 1: "player-1" "r,," "2" "4" "6"
	//	verify server send to client 2: "r,," "1" "3" "5" "w7"
	{
		list<string> client1Send({ ",", "1", "3", "5", "w7" });
		list<string> client2Send({ ",", "2", "4", "6" });
		expectedSendToClient1Msgs = { "player-1", "r,,", "2", "4", "6" };
		expectedSendToClient2Msgs = { "r,,", "1", "3", "5", "w7" };
		c.setClient1SentMsgs(client1Send);
		c.setClient2SentMsgs(client2Send);
		s.run();
		sendToClient1Msgs = &c.getSendToClient1Msgs();
		sendToClient2Msgs = &c.getSendToClient2Msgs();
		for (size_t k = 0; k < sendToClient1Msgs->size(); k++)
			assert(sendToClient1Msgs->at(k) ==
					expectedSendToClient1Msgs[k]);
		for (size_t k = 0; k < sendToClient2Msgs->size(); k++)
			assert(sendToClient2Msgs->at(k) ==
					expectedSendToClient2Msgs[k]);
		c.clearSendToClient1Msgs();
		c.clearSendToClient2Msgs();
	}

	// test client 1, client 2 guest and client 2 win
	// client 1 send: "," "2" "4" "6" "8"
	// client 2 send: "," "1" "3" "5" "w7"
	//	verify server send to client 1: "player-1" "r,," "2" "4" "6"
	//	verify server send to client 2: "r,," "1" "3" "5" "w7"
	{
		list<string> client1Send({ ",", "2", "4", "6", "8" });
		list<string> client2Send({ ",", "1", "3", "5", "w7" });
		expectedSendToClient1Msgs = { "player-1", "r,,", "1", "3", "5", "w7" };
		expectedSendToClient2Msgs = { "r,,", "2", "4", "6", "8" };
		c.setClient1SentMsgs(client1Send);
		c.setClient2SentMsgs(client2Send);
		s.run();
		sendToClient1Msgs = &c.getSendToClient1Msgs();
		sendToClient2Msgs = &c.getSendToClient2Msgs();
		for (size_t k = 0; k < sendToClient1Msgs->size(); k++)
			assert(sendToClient1Msgs->at(k) ==
					expectedSendToClient1Msgs[k]);
		for (size_t k = 0; k < sendToClient2Msgs->size(); k++)
			assert(sendToClient2Msgs->at(k) ==
					expectedSendToClient2Msgs[k]);
		c.clearSendToClient1Msgs();
		c.clearSendToClient2Msgs();
	}

	// test client 1, client 2 guest and client 1 giveup
	// client 1 send: "," "2" "4" "6" "giveup"
	// client 2 send: "," "1" "3" "5"
	//	verify server send to client 1: "player-1" "r,," "1" "3" "5"
	//	verify server send to client 2: "r,," "2" "4" "6" "giveup"
	{
		list<string> client1Send({ ",", "2", "4", "6", "giveup" });
		list<string> client2Send({ ",", "1", "3", "5" });
		expectedSendToClient1Msgs = { "player-1", "r,,", "1", "3", "5" };
		expectedSendToClient2Msgs = { "r,,", "2", "4", "6", "giveup" };
		c.setClient1SentMsgs(client1Send);
		c.setClient2SentMsgs(client2Send);
		s.run();
		sendToClient1Msgs = &c.getSendToClient1Msgs();
		sendToClient2Msgs = &c.getSendToClient2Msgs();
		for (size_t k = 0; k < sendToClient1Msgs->size(); k++)
			assert(sendToClient1Msgs->at(k) ==
					expectedSendToClient1Msgs[k]);
		for (size_t k = 0; k < sendToClient2Msgs->size(); k++)
			assert(sendToClient2Msgs->at(k) ==
					expectedSendToClient2Msgs[k]);
		c.clearSendToClient1Msgs();
		c.clearSendToClient2Msgs();
	}

	// test client 1, client 2 guest and client 2 giveup
	// client 1 send: "," "1" "3" "5"
	// client 2 send: "," "2" "4" "6" "giveup"
	//	verify server send to client 1: "player-1" "r,," "2" "4" "6" "giveup"
	//	verify server send to client 2: "r,," "1" "3" "5"
	{
		list<string> client1Send({ ",", "1", "3", "5" });
		list<string> client2Send({ ",", "2", "4", "6", "giveup" });
		expectedSendToClient1Msgs = { "player-1", "r,,", "2", "4", "6", "giveup" };
		expectedSendToClient2Msgs = { "r,,", "1", "3", "5" };
		c.setClient1SentMsgs(client1Send);
		c.setClient2SentMsgs(client2Send);
		s.run();
		sendToClient1Msgs = &c.getSendToClient1Msgs();
		sendToClient2Msgs = &c.getSendToClient2Msgs();
		for (size_t k = 0; k < sendToClient1Msgs->size(); k++)
			assert(sendToClient1Msgs->at(k) ==
					expectedSendToClient1Msgs[k]);
		for (size_t k = 0; k < sendToClient2Msgs->size(); k++)
			assert(sendToClient2Msgs->at(k) ==
					expectedSendToClient2Msgs[k]);
		c.clearSendToClient1Msgs();
		c.clearSendToClient2Msgs();
	}

	// test client 1, client 2 guest and client 1 bye
	// client 1 send: "," "2" "4" "6" "bye"
	// client 2 send: "," "1" "3" "5"
	//	verify server send to client 1: "player-1" "r,," "1" "3" "5"
	//	verify server send to client 2: "r,," "2" "4" "6" "bye"
	{
		list<string> client1Send({ ",", "2", "4", "6", "bye" });
		list<string> client2Send({ ",", "1", "3", "5" });
		expectedSendToClient1Msgs = { "player-1", "r,,", "1", "3", "5" };
		expectedSendToClient2Msgs = { "r,,", "2", "4", "6", "bye" };
		c.setClient1SentMsgs(client1Send);
		c.setClient2SentMsgs(client2Send);
		s.run();
		sendToClient1Msgs = &c.getSendToClient1Msgs();
		sendToClient2Msgs = &c.getSendToClient2Msgs();
		for (size_t k = 0; k < sendToClient1Msgs->size(); k++)
			assert(sendToClient1Msgs->at(k) ==
					expectedSendToClient1Msgs[k]);
		for (size_t k = 0; k < sendToClient2Msgs->size(); k++)
			assert(sendToClient2Msgs->at(k) ==
					expectedSendToClient2Msgs[k]);
		c.clearSendToClient1Msgs();
		c.clearSendToClient2Msgs();
	}

	// test client 1, client 2 guest and client 2 bye
	// client 1 send: "," "1" "3" "5"
	// client 2 send: "," "2" "4" "6" "bye"
	//	verify server send to client 1: "player-1" "r,," "2" "4" "6" "bye"
	//	verify server send to client 2: "r,," "1" "3" "5"
	{
		list<string> client1Send({ ",", "1", "3", "5" });
		list<string> client2Send({ ",", "2", "4", "6", "bye" });
		expectedSendToClient1Msgs = { "player-1", "r,,", "2", "4", "6", "bye" };
		expectedSendToClient2Msgs = { "r,,", "1", "3", "5" };
		c.setClient1SentMsgs(client1Send);
		c.setClient2SentMsgs(client2Send);
		s.run();
		sendToClient1Msgs = &c.getSendToClient1Msgs();
		sendToClient2Msgs = &c.getSendToClient2Msgs();
		for (size_t k = 0; k < sendToClient1Msgs->size(); k++)
			assert(sendToClient1Msgs->at(k) ==
					expectedSendToClient1Msgs[k]);
		for (size_t k = 0; k < sendToClient2Msgs->size(); k++)
			assert(sendToClient2Msgs->at(k) ==
					expectedSendToClient2Msgs[k]);
		c.clearSendToClient1Msgs();
		c.clearSendToClient2Msgs();
	}

	// test client 1, client 2 non-guests and client 2 bye
	// client 1 send: "a,a" "1" "3" "5"
	// client 2 send: "b,b" "2" "4" "6" "bye"
	//	verify server send to client 1: "player-1" "r0,0,b" "2" "4" "6" "bye"
	//	verify server send to client 2: "r0,0,a" "1" "3" "5"
	{
		list<string> client1Send({ "a,a", "1", "3", "5" });
		list<string> client2Send({ "b,b", "2", "4", "6", "bye" });
		expectedSendToClient1Msgs = { "player-1", "r0,0,b", "2", "4", "6", "bye" };
		expectedSendToClient2Msgs = { "r0,0,a", "1", "3", "5" };
		c.setClient1SentMsgs(client1Send);
		c.setClient2SentMsgs(client2Send);
		s.run();
		sendToClient1Msgs = &c.getSendToClient1Msgs();
		sendToClient2Msgs = &c.getSendToClient2Msgs();
		for (size_t k = 0; k < sendToClient1Msgs->size(); k++)
			assert(sendToClient1Msgs->at(k) ==
					expectedSendToClient1Msgs[k]);
		for (size_t k = 0; k < sendToClient2Msgs->size(); k++)
			assert(sendToClient2Msgs->at(k) ==
					expectedSendToClient2Msgs[k]);
		c.clearSendToClient1Msgs();
		c.clearSendToClient2Msgs();
	}

	// TODO:
	// test client 1 non-guest, client 2 guest and client 2 bye
	// client 1 send: "a,a" "1" "3" "5"
	// client 2 send: "b,b" "2" "4" "6" "bye"
	//	verify server send to client 1: "player-1" "r0,0,b" "2" "4" "6" "bye"
	//	verify server send to client 2: "r0,0,a" "1" "3" "5"
/*	{
		list<string> client1Send({ "a,a", "1", "3", "5" });
		list<string> client2Send({ "b,b", "2", "4", "6", "bye" });
		expectedSendToClient1Msgs = { "player-1", "r0,0,b", "2", "4", "6", "bye" };
		expectedSendToClient2Msgs = { "r0,0,a", "1", "3", "5" };
		c.setClient1SentMsgs(client1Send);
		c.setClient2SentMsgs(client2Send);
		s.run();
		sendToClient1Msgs = &c.getSendToClient1Msgs();
		sendToClient2Msgs = &c.getSendToClient2Msgs();
		for (size_t k = 0; k < sendToClient1Msgs->size(); k++)
			assert(sendToClient1Msgs->at(k) ==
					expectedSendToClient1Msgs[k]);
		for (size_t k = 0; k < sendToClient2Msgs->size(); k++)
			assert(sendToClient2Msgs->at(k) ==
					expectedSendToClient2Msgs[k]);
		c.clearSendToClient1Msgs();
		c.clearSendToClient2Msgs();
	}
*/
	// TODO:
	// test client 1 guest, client 2 non-guest and client 2 bye
	// client 1 send: "a,a" "1" "3" "5"
	// client 2 send: "b,b" "2" "4" "6" "bye"
	//	verify server send to client 1: "player-1" "r0,0,b" "2" "4" "6" "bye"
	//	verify server send to client 2: "r0,0,a" "1" "3" "5"
/*	{
		list<string> client1Send({ "a,a", "1", "3", "5" });
		list<string> client2Send({ "b,b", "2", "4", "6", "bye" });
		expectedSendToClient1Msgs = { "player-1", "r0,0,b", "2", "4", "6", "bye" };
		expectedSendToClient2Msgs = { "r0,0,a", "1", "3", "5" };
		c.setClient1SentMsgs(client1Send);
		c.setClient2SentMsgs(client2Send);
		s.run();
		sendToClient1Msgs = &c.getSendToClient1Msgs();
		sendToClient2Msgs = &c.getSendToClient2Msgs();
		for (size_t k = 0; k < sendToClient1Msgs->size(); k++)
			assert(sendToClient1Msgs->at(k) ==
					expectedSendToClient1Msgs[k]);
		for (size_t k = 0; k < sendToClient2Msgs->size(); k++)
			assert(sendToClient2Msgs->at(k) ==
					expectedSendToClient2Msgs[k]);
		c.clearSendToClient1Msgs();
		c.clearSendToClient2Msgs();
	}
*/
}

int main()
{
	testParentServer();
	cout << "All Parent Server tests passed!" << endl;

	testChildServer();
	cout << "All Child Server tests passed!" << endl;
}
