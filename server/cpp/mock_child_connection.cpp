#include "mock_child_connection.h"
#include "exceptions.h"
#include <string>
using namespace std;

string MockChildConnection::receiveFromClient1(int time)
{
	if (m_client1SentMsgs.empty())
		throw runtime_error("m_client1SentMsgs is empty.");

	string toReturn = m_client1SentMsgs.front();
	m_client1SentMsgs.pop_front();
	return toReturn;
}

void MockChildConnection::sendToClient1(string text)
{
	m_sendToClient1Msgs.push_back(text);
}

string MockChildConnection::receiveFromClient2(int time)
{
	if (m_client2SentMsgs.empty())
		throw runtime_error("m_client2SentMsgs is empty.");

	string toReturn = m_client2SentMsgs.front();
	m_client2SentMsgs.pop_front();
	return toReturn;
}

void MockChildConnection::sendToClient2(string text)
{
	m_sendToClient2Msgs.push_back(text);
}
