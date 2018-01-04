#ifndef CHILD_CONNECTION_H
#define CHILD_CONNECTION_H

#include <string>

class ChildConnection
{
public:
	virtual ~ChildConnection() {}

	// client 1
	virtual int getClient1Port() const = 0;
	virtual std::string getClient1IP() const = 0;
	virtual std::string receiveFromClient1(int time) = 0;
	virtual void sendToClient1(std::string text) = 0;
	virtual void acceptClient1(int time = 0) = 0;
	virtual void closeClient1() = 0;

	// client 2
	virtual int getClient2Port() const = 0;
	virtual std::string getClient2IP() const = 0;
	virtual std::string receiveFromClient2(int time) = 0;
	virtual void sendToClient2(std::string text) = 0;
	virtual void acceptClient2(int time = 0) = 0;
	virtual void closeClient2() = 0;
};

#endif  // CHILD_CONNECTION_H
