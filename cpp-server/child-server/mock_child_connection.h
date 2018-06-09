#ifndef MOCK_CHILD_CONNECTION_H
#define MOCK_CHILD_CONNECTION_H

#include "child_connection.h"
#include <list>
#include <string>
#include <vector>

class MockChildConnection : public ChildConnection
{
public:
	virtual ~MockChildConnection() {}

	// client 1
	virtual int getClient1Port() const { return 0; }
	virtual std::string getClient1IP() const { return ""; }
	virtual std::string receiveFromClient1(int time);
	virtual void sendToClient1(std::string text);
	virtual void acceptClient1(int time = 0) {}
	virtual void closeClient1() {}
	const std::vector<std::string>& getSendToClient1Msgs() const
	{
		return m_sendToClient1Msgs;
	}
	void setClient1SentMsgs(const std::list<std::string>& sentMsgs)
	{
		m_client1SentMsgs = sentMsgs;
	}
	void clearSendToClient1Msgs()
	{
		m_sendToClient1Msgs.clear();
	}

	// client 2
	virtual int getClient2Port() const { return 0; }
	virtual std::string getClient2IP() const { return ""; }
	virtual std::string receiveFromClient2(int time);
	virtual void sendToClient2(std::string text);
	virtual void acceptClient2(int time = 0) {}
	virtual void closeClient2() {}
	const std::vector<std::string>& getSendToClient2Msgs() const
	{
		return m_sendToClient2Msgs;
	}
	void setClient2SentMsgs(const std::list<std::string>& sentMsgs)
	{
		m_client2SentMsgs = sentMsgs;
	}
	void clearSendToClient2Msgs()
	{
		m_sendToClient2Msgs.clear();
	}

private:
	std::list<std::string> m_client1SentMsgs;
	std::vector<std::string> m_sendToClient1Msgs;
	
	std::list<std::string> m_client2SentMsgs;
	std::vector<std::string> m_sendToClient2Msgs;
};

#endif  // MOCK_CHILD_CONNECTION_H
