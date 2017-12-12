#ifndef MOCK_CONNECTION_H
#define MOCK_CONNECTION_H

#include "connection.h"
#include <string>
#include <vector>
#include <list>

class MockConnection : public Connection
{
public:
	virtual ~MockConnection() {}
	virtual int getClientPort() const { return 0; }
	virtual std::string getClientIP() const { return ""; }
	virtual std::string receiveFrom(int time);
	virtual void sendTo(std::string text);
	virtual void acceptClient(int time = 0) {}
	virtual void closeClient() {}
	const std::vector<std::string>& getSentMsgs() const
	{
		return m_sentMsgs;
	}
	void setReceivedMsgs(const std::list<std::string>& receivedMsgs)
	{
		m_receivedMsgs = receivedMsgs;
	}

private:
	std::list<std::string> m_receivedMsgs;
	std::vector<std::string> m_sentMsgs;
};

#endif  // MOCK_CONNECTION_H
