#ifndef CHILD_CONNECTION_H
#define CHILD_CONNECTION_H

#include <string>

class ChildConnection
{
public:
	virtual ~ChildConnection() {}
	virtual int getClientPort() const = 0;
	virtual std::string getClientIP() const = 0;
	virtual std::string receiveFrom(int time) = 0;
	virtual void sendTo(std::string text) = 0;
	virtual void acceptClient(int time = 0) = 0;
	virtual void closeClient() = 0;
};

#endif  // CHILD_CONNECTION_H
