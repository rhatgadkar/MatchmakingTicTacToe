#ifndef MOCK_CHILD_CONNECTION_H
#define MOCK_CHILD_CONNECTION_H

#include "child_connection.h"

class MockChildConnection : public ChildConnection
{
public:
	virtual ~MockChildConnection();

	// client 1
	virtual int getClient1Port() const { return 0; }
	virtual std::string getClient1IP() const { return ""; }
	virtual std::string receiveFromClient1(int time);
	virtual void sendToClient1(std::string text);
	virtual void acceptClient1(int time = 0) {}
	virtual void closeClient1() {}

	// client 2
	virtual int getClient2Port() const { return 0; }
	virtual std::string getClient2IP() const { return ""; }
	virtual std::string receiveFromClient2(int time);
	virtual void sendToClient2(std::string text);
	virtual void acceptClient2(int time = 0) {}
	virtual void closeClient2() {}
};

#endif  // MOCK_CHILD_CONNECTION_H
