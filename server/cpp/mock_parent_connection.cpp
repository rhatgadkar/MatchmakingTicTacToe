#include "mock_parent_connection.h"
#include "exceptions.h"
#include <vector>
#include <list>
#include <string>
using namespace std;

string MockParentConnection::receiveFrom(int time)
{
	if (m_receivedMsgs.empty())
		throw runtime_error("m_receivedMsgs is empty.");

	string toReturn = m_receivedMsgs.front();
	m_receivedMsgs.pop_front();
	return toReturn;
}

void MockParentConnection::sendTo(string text)
{
	m_sentMsgs.push_back(text);
}
