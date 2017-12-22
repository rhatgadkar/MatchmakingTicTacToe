#include "child_server.h"
#include "write_named_pipe.h"
#include "utilities.h"
#include "constants.h"
#include "db-accessor.h"
#include "exceptions.h"
#include <string>
#include <iostream>
#include <cstring>
#include <pthread.h>
using namespace std;

void ChildServer::run()
{
	// accept client 1
	try
	{
		m_childConnection.acceptClient1(15);
	}
	catch (...)
	{
		cout << "Closing child server." << endl;
		return;
	}

	// receive client 1 login
	string login;
	try
	{
		login = m_childConnection.receiveFromClient1(15);
	}
	catch (...)
	{
		cout << "Did not receive login info. CLosing child server."
			<< endl;
		return;
	}
	try
	{
		setClient1LoginProvided(login);
	}
	catch (IncorrectLoginException)
	{
		cout << "Incorrect login. Closing child server." << endl;
		try
		{
			// send 'invalidl' to client 1
			m_childConnection.sendToClient1("invalidl");
		}
		catch (...)
		{
			cerr << "Error sending 'invalidl' to client 1." << endl;
		}
		return;
	}
	catch (UserInGameException)
	{
		cout << "User is in game. Closing child server." << endl;
		try
		{
			// send 'loggedin' to client 1
			m_childConnection.sendToClient1("loggedin");
		}
		catch (...)
		{
			cerr << "Error sending 'loggedin' to client 1."
				<< endl;
		}
		return;
	}
	cout << "client 1 success" << endl;


	// send player 1 ACK to client 1
	try
	{
		m_childConnection.sendToClient1("player-1");
	}
	catch (...)
	{
		cerr << "Error sending player 1 ACK to client 1" << endl;
		return;
	}
	cout << "First client connected: " << m_childConnection.getClient1IP()
		<< endl;

	// wait to accept client 2
	cout << "Waiting for second client to connect..." << endl;
	for (;;)
	{
		bool client2Connected = isClient2Connected();
		if (!client2Connected)
		{
			m_childConnection.closeClient1();
			return;
		}

		// receive client 2 login
		try
		{
			login = m_childConnection.receiveFromClient2(15);
		}
		catch (...)
		{
			cout << "Did not receive login info. "
				<< "Waiting for new client." << endl;
			m_childConnection.closeClient2();
			continue;
		}
		try
		{
			setClient2LoginProvided(login);
		}
		catch (IncorrectLoginException)
		{
			cout << "Incorrect login. "
				<< "Waiting for new client." << endl;
			try
			{
				// send 'invalidl' to client 2
				m_childConnection.sendToClient2("invalidl");
			}
			catch (...)
			{
				cerr << "Error sending 'invalidl' to client 2." << endl;
			}
			m_childConnection.closeClient2();
			continue;
		}
		catch (UserInGameException)
		{
			cout << "User is in game. "
				<< "Waiting for new client." << endl;
			try
			{
				// send 'loggedin' to client 2
				m_childConnection.sendToClient2("loggedin");
			}
			catch (...)
			{
				cerr << "Error sending 'loggedin' to client 2."
					<< endl;
			}
			m_childConnection.closeClient2();
			continue;
		}
		cout << "client 2 success" << endl;

		// if login provided, get win/loss records and send to clients
		setClient1WinLossMsg();
		setClient2WinLossMsg();
		try
		{
			cout << "Sending to client 1: " << m_client1WinLossMsg
				<< endl;
			m_childConnection.sendToClient1(m_client1WinLossMsg);
		}
		catch (...)
		{
			cerr << "Error sending message to client 1." << endl;
			m_childConnection.closeClient2();
			continue;
		}
		try
		{
			cout << "Sending to client 2: " << m_client2WinLossMsg
				<< endl;
			m_childConnection.sendToClient2(m_client2WinLossMsg);
		}
		catch (...)
		{
			cerr << "Error sending message to client 2." << endl;
			m_childConnection.closeClient2();
			continue;
		}
		
		break;
	}
	cout << "Second client connected: " << m_childConnection.getClient2IP()
		<< endl;

	// start match threads for client 1 and client 2
}

void* ChildServer::receiveDisconnectClient1Thread(void* args)
{
	Client2ConnectedThreadArgs* a = (Client2ConnectedThreadArgs*)a;
	bool* messageReceived = a->messageReceived;
	bool* client2AcceptExpired = a->client2AcceptExpired;
	ChildServer* cs = a->childServer;

	while (!client2AcceptExpired)
	{
		string msg;
		try
		{
			msg = cs->m_childConnection.receiveFromClient1(1);
			*messageReceived = true;
			break;
		}
		catch (...)
		{
		}
	}

	return NULL;
}

bool ChildServer::isClient2Connected()
{
	// start thread to receive messages from client 1.
	// If message received, abort waiting for client 2 to connect.
	bool messageReceived = false;
	bool client2AcceptExpired = false;
	Client2ConnectedThreadArgs a;
	a.messageReceived = &messageReceived;
	a.client2AcceptExpired = &client2AcceptExpired;
	a.childServer = this;
	pthread_t receiveClient1Thread;
	pthread_create(&receiveClient1Thread, NULL,
			&ChildServer::receiveDisconnectClient1Thread, &a);

	// try to accept client 2 within 15 seconds
	bool client2Accepted = false;
	for (int time = 0; time < 15 && !messageReceived; time++)
	{
		try
		{
			m_childConnection.acceptClient2(1);
			client2Accepted = true;
			break;
		}
		catch (...)
		{
		}
	}
	client2AcceptExpired = true;
	pthread_join(receiveClient1Thread, NULL);
	if (client2Accepted)
		return true;
	return false;
}

void ChildServer::setClient1LoginProvided(const string& login)
{
	if (login[0] == ',')
	{
		m_client1LoginProvided = false;
		return;
	}

	char username_c[MAXBUFLEN];
	char password_c[MAXBUFLEN];
	int status;

	memset(username_c, 0, MAXBUFLEN);
	memset(password_c, 0, MAXBUFLEN);

	get_login_info(login.c_str(), username_c, password_c);
	status = is_login_valid(username_c, password_c);
	if (status == 0)
		throw IncorrectLoginError;
	else if (status == -1)
		throw UserInGameError;

	m_client1Username = username_c;
	m_client1Password = password_c;
	m_client1LoginProvided = true;
}

void ChildServer::setClient1WinLossMsg()
{
	if (!m_client1LoginProvided)
	{
		m_client1WinLossMsg = "r,,";
		return;
	}

	char win_c[MAXBUFLEN];
	char loss_c[MAXBUFLEN];
	string win;
	string loss;

	memset(win_c, 0, MAXBUFLEN);
	memset(loss_c, 0, MAXBUFLEN);
	get_win_loss_record(m_client1Username.c_str(), win_c, loss_c);
	win = win_c;
	loss = loss_c;

	m_client1WinLossMsg = "r" + win + "," + loss + ",";
	if (m_client2Username != "")
		m_client1WinLossMsg += m_client2Username;
}

void ChildServer::setClient2LoginProvided(const string& login)
{
	if (login[0] == ',')
	{
		m_client2LoginProvided = false;
		return;
	}

	char username_c[MAXBUFLEN];
	char password_c[MAXBUFLEN];
	int status;

	memset(username_c, 0, MAXBUFLEN);
	memset(password_c, 0, MAXBUFLEN);

	get_login_info(login.c_str(), username_c, password_c);
	status = is_login_valid(username_c, password_c);
	if (status == 0)
		throw IncorrectLoginError;
	else if (status == -1)
		throw UserInGameError;

	m_client2Username = username_c;
	m_client2Password = password_c;
	m_client2LoginProvided = true;
}

void ChildServer::setClient2WinLossMsg()
{
	if (!m_client2LoginProvided)
	{
		m_client2WinLossMsg = "r,,";
		return;
	}

	char win_c[MAXBUFLEN];
	char loss_c[MAXBUFLEN];
	string win;
	string loss;

	memset(win_c, 0, MAXBUFLEN);
	memset(loss_c, 0, MAXBUFLEN);
	get_win_loss_record(m_client2Username.c_str(), win_c, loss_c);
	win = win_c;
	loss = loss_c;

	m_client2WinLossMsg = "r" + win + "," + loss + ",";
	if (m_client1Username != "")
		m_client2WinLossMsg += m_client1Username;
}

ChildServer::~ChildServer()
{
	WriteNamedPipe writeNamedPipe(false);
	string portStr = intToStr(m_port);
	writeNamedPipe.writePipe(portStr, portStr.length());
}
