#ifndef MOCK_PARENT_CONNECTION_H
#define MOCK_PARENT_CONNECTION_H

#include "parent_connection.h"
#include <string>
#include <vector>
#include <list>

class MockParentConnection : public ParentConnection
{
public:
	virtual ~MockParentConnection() {}
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
	void clearSentMsgs()
	{
		m_sentMsgs.clear();
	}

private:
	std::list<std::string> m_receivedMsgs;
	std::vector<std::string> m_sentMsgs;
};

#endif  // MOCK_PARENT_CONNECTION_H
