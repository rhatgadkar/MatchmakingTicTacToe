#include "mock_child_connection.h"
#include "exceptions.h"
#include <string>
using namespace std;

string MockChildConnection::receiveFromClient1(int time)
{
	if (m_client1ReceivedMsgs.empty())
		throw runtime_error("m_receivedMsgs is empty.");

	string toReturn = m_client1ReceivedMsgs.front();
	m_client1ReceivedMsgs.pop_front();
	return toReturn;
}

void MockChildConnection::sendToClient1(string text)
{
	m_client1SentMsgs.push_back(text);
}

string MockChildConnection::receiveFromClient2(int time)
{
	if (m_client2ReceivedMsgs.empty())
		throw runtime_error("m_receivedMsgs is empty.");

	string toReturn = m_client2ReceivedMsgs.front();
	m_client2ReceivedMsgs.pop_front();
	return toReturn;
}

void MockChildConnection::sendToClient2(string text)
{
	m_client2SentMsgs.push_back(text);
}
