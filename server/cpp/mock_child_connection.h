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
	const std::vector<std::string>& getClient1SentMsgs() const
	{
		return m_client1SentMsgs;
	}
	void setClient1ReceivedMsgs(const std::list<std::string>& receivedMsgs)
	{
		m_client1ReceivedMsgs = receivedMsgs;
	}
	void clearClient1SentMsgs()
	{
		m_client1SentMsgs.clear();
	}

	// client 2
	virtual int getClient2Port() const { return 0; }
	virtual std::string getClient2IP() const { return ""; }
	virtual std::string receiveFromClient2(int time);
	virtual void sendToClient2(std::string text);
	virtual void acceptClient2(int time = 0) {}
	virtual void closeClient2() {}
	const std::vector<std::string>& getClient2SentMsgs() const
	{
		return m_client2SentMsgs;
	}
	void setClient2ReceivedMsgs(const std::list<std::string>& receivedMsgs)
	{
		m_client2ReceivedMsgs = receivedMsgs;
	}
	void clearClient2SentMsgs()
	{
		m_client2SentMsgs.clear();
	}

private:
	std::list<std::string> m_client1ReceivedMsgs;
	std::vector<std::string> m_client1SentMsgs;
	
	std::list<std::string> m_client2ReceivedMsgs;
	std::vector<std::string> m_client2SentMsgs;
};

#endif  // MOCK_CHILD_CONNECTION_H
