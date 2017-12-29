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
		m_client1.setLoginProvided(login);
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
	do
	{
		bool client2Connected = isClient2Connected();
		if (!client2Connected)
		{
			m_childConnection.closeClient1();
			cout << "No opponent found for this server." << endl;
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
			m_client2.setLoginProvided(login);
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
		break;
	} while (FOREVER);
	cout << "client 2 success" << endl;

	// if login provided, get win/loss records and send to clients
	m_client1.setWinLossMsg();
	m_client2.setWinLossMsg();
	try
	{
		cout << "Sending to client 1: " << m_client1.getWinLossMsg()
			<< endl;
		m_childConnection.sendToClient1(m_client1.getWinLossMsg());
	}
	catch (...)
	{
		cerr << "Error sending message to client 1." << endl;
		m_childConnection.closeClient2();
		return;
	}
	try
	{
		cout << "Sending to client 2: " << m_client2.getWinLossMsg()
			<< endl;
		m_childConnection.sendToClient2(m_client2.getWinLossMsg());
	}
	catch (...)
	{
		cerr << "Error sending message to client 2." << endl;
		m_childConnection.closeClient2();
		return;
	}
	cout << "Second client connected: " << m_childConnection.getClient2IP()
		<< endl;

	// start match threads for client 1 and client 2
	pthread_t client1MatchThread;
	pthread_t client2MatchThread;
	char client1Record = 'n';
	char client2Record = 'n';
	pthread_mutex_t clientRecordMutex;
	MatchThreadArgs childMatchThreadArgs;
	childMatchThreadArgs.client1Record = &client1Record;
	childMatchThreadArgs.client2Record = &client2Record;
	childMatchThreadArgs.clientRecordMutex = &clientRecordMutex;
	childMatchThreadArgs.childServer = this;
	pthread_create(&client1MatchThread, NULL,
			&ChildServer::client1MatchThread,
			&childMatchThreadArgs);
	pthread_create(&client2MatchThread, NULL,
			&ChildServer::client2MatchThread,
			&childMatchThreadArgs);
	pthread_join(client1MatchThread, NULL);
	pthread_join(client2MatchThread, NULL);

	if (FOREVER)
	{
		// set both clients to not in game in database
		if (m_client1.isLoginProvided())
			set_user_no_ingame(m_client1.getUsername().c_str());
		if (m_client2.isLoginProvided())
			set_user_no_ingame(m_client2.getUsername().c_str());
	}

	// find out who won/loss and update database
	if (client2Record == 'w')
	{
		cout << "Client 1 lost." << endl;
		cout << "Client 2 won." << endl;
		if (FOREVER)
			update_win_loss_record(m_client1.getUsername().c_str(),
					'l', m_client2.getUsername().c_str(),
					'w');
	}
	else if (client1Record == 'w')
	{
		cout << "Client 1 won." << endl;
		cout << "Client 2 lost." << endl;
		if (FOREVER)
			update_win_loss_record(m_client1.getUsername().c_str(),
					'w', m_client2.getUsername().c_str(),
					'l');
	}
	else if (client2Record == 'g')
	{
		cout << "Client 1 won." << endl;
		cout << "Client 2 lost." << endl;
		if (FOREVER)
			update_win_loss_record(m_client1.getUsername().c_str(),
					'w', m_client2.getUsername().c_str(),
					'l');
	}
	else if (client1Record == 'g')
	{
		cout << "Client 1 lost." << endl;
		cout << "Client 2 won." << endl;
		if (FOREVER)
			update_win_loss_record(m_client1.getUsername().c_str(),
					'l', m_client2.getUsername().c_str(),
					'w');
	}

	// game is over so close connections to clients
	m_childConnection.closeClient1();
	m_childConnection.closeClient2();
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
		}
		catch (TimeoutException)
		{
			continue;
		}
		catch (...)
		{
		}
		*messageReceived = true;
		break;
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

ChildServer::~ChildServer()
{
	WriteNamedPipe writeNamedPipe(false);
	string portStr = intToStr(m_port);
	writeNamedPipe.writePipe(portStr, portStr.length());
}

void ChildServer::Client::setLoginProvided(const std::string& login)
{
	if (login[0] == ',')
	{
		m_loginProvided = false;
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

	m_username = username_c;
	m_password = password_c;
	m_loginProvided = true;
}

void ChildServer::Client::setWinLossMsg()
{
	if (!m_loginProvided)
	{
		m_winLossMsg = "r,,";
		return;
	}

	char win_c[MAXBUFLEN];
	char loss_c[MAXBUFLEN];
	string win;
	string loss;

	if (FOREVER)
	{
		memset(win_c, 0, MAXBUFLEN);
		memset(loss_c, 0, MAXBUFLEN);
		get_win_loss_record(m_username.c_str(), win_c, loss_c);
		win = win_c;
		loss = loss_c;
	}
	else
	{
		win = "0";
		loss = "0";
	}

	m_winLossMsg = "r" + win + "," + loss + ",";
	if (m_username != "")
		m_winLossMsg += m_username;
}

void* ChildServer::client1MatchThread(void* args)
{
	MatchThreadArgs* a = (MatchThreadArgs*)args;
	char* client1Record = (char*)a->client1Record;
	char* client2Record = (char*)a->client2Record;
	pthread_mutex_t* clientRecordMutex =
		(pthread_mutex_t*)a->clientRecordMutex;
	ChildServer* cs = a->childServer;

	do
	{
		string msg;
		try
		{
			msg = cs->m_childConnection.receiveFromClient1(40);
		}
		catch (DisconnectException)
		{
			cout << "Received 'giveup'" << endl;
			// send 'giveup' to second address only if no record
			// has been set previously for other client and break
			pthread_mutex_lock(clientRecordMutex);
			if (*client1Record == 'n')
			{
				*client2Record = 'g';
				pthread_mutex_unlock(clientRecordMutex);
				try
				{
					cs->m_childConnection.sendToClient2("giveup");
				}
				catch (...)
				{
					cerr << "Error in sending 'giveup' to "
						<< "client 2" << endl;
				}
			}
			break;
		}
		catch (TimeoutException)
		{
			cout << "Not receiving anything. Closing child server."
				<< endl;
			// send 'bye' to client 1 and client 2
			try
			{
				cs->m_childConnection.sendToClient2("bye");
			}
			catch (...)
			{
				cerr << "Error in sending 'bye' to client 2."
					<< endl;
			}
			try
			{
				cs->m_childConnection.sendToClient1("bye");
			}
			catch (...)
			{
				cerr << "Error in sending 'bye' to client 1."
					<< endl;
			}
			break;
		}
		catch (...)
		{
			cerr << "Error in receiving message from client 1."
				<< endl;
			break;
		}
		cout << "Receiving message from client 1: "
			<< cs->m_childConnection.getClient1IP() << ": " << msg
			<< endl;

		if (msg == "bye")
		{
			cout << "Received 'bye', closing connection to "
				<< "client 1: "
				<< cs->m_childConnection.getClient1IP()
				<< endl;
			// send bye to client 2
			try
			{
				cs->m_childConnection.sendToClient2("bye");
			}
			catch (...)
			{
				cerr << "Error in sending 'bye' to client 2"
					<< endl;
			}
			break;
		}
		else if (msg == "giveup")
		{
			cout << "Received 'giveup'" << endl;
			// send 'giveup' to second address only if no record
			// has been set previously for other client and break
			pthread_mutex_lock(clientRecordMutex);
			if (*client1Record == 'n')
			{
				*client2Record = 'g';
				pthread_mutex_unlock(clientRecordMutex);
				try
				{
					cs->m_childConnection.sendToClient2("giveup");
				}
				catch (...)
				{
					cerr << "Error in sending 'giveup' to "
						<< "client 2" << endl;
				}
			}
			break;
		}
		else
		{
			// forward message to client 2
			if (msg[0] == 'w')
			{
				pthread_mutex_lock(clientRecordMutex);
				if (*client1Record == 'n')
					*client2Record = 'w';
				pthread_mutex_unlock(clientRecordMutex);
			}
			else if (msg[0] == 't')
			{
				pthread_mutex_lock(clientRecordMutex);
				if (*client1Record == 'n')
					*client2Record = 't';
				pthread_mutex_unlock(clientRecordMutex);
			}
			try
			{
				cs->m_childConnection.sendToClient2(msg);
				cout << "Forwarding message from client 1: "
					<< cs->m_childConnection.getClient1IP()
					<< endl;
			}
			catch (...)
			{
				cerr << "Error forwarding message to client 2."
					<< endl;
				break;
			}
			if (msg[0] == 'w' || msg[0] == 't')
				break;
		}
	} while (FOREVER);

	return NULL;
}

void* ChildServer::client2MatchThread(void* args)
{
	MatchThreadArgs* a = (MatchThreadArgs*)args;
	char* client1Record = (char*)a->client1Record;
	char* client2Record = (char*)a->client2Record;
	pthread_mutex_t* clientRecordMutex =
		(pthread_mutex_t*)a->clientRecordMutex;
	ChildServer* cs = a->childServer;

	do
	{
		string msg;
		try
		{
			msg = cs->m_childConnection.receiveFromClient2(40);
		}
		catch (DisconnectException)
		{
			cout << "Received 'giveup'" << endl;
			// send 'giveup' to second address only if no record
			// has been set previously for other client and break
			pthread_mutex_lock(clientRecordMutex);
			if (*client1Record == 'n')
			{
				*client2Record = 'g';
				pthread_mutex_unlock(clientRecordMutex);
				try
				{
					cs->m_childConnection.sendToClient1("giveup");
				}
				catch (...)
				{
					cerr << "Error in sending 'giveup' to "
						<< "client 1" << endl;
				}
			}
			break;
		}
		catch (TimeoutException)
		{
			cout << "Not receiving anything. Closing child server."
				<< endl;
			// send 'bye' to client 1 and client 2
			try
			{
				cs->m_childConnection.sendToClient1("bye");
			}
			catch (...)
			{
				cerr << "Error in sending 'bye' to client 1."
					<< endl;
			}
			try
			{
				cs->m_childConnection.sendToClient2("bye");
			}
			catch (...)
			{
				cerr << "Error in sending 'bye' to client 2."
					<< endl;
			}
			break;
		}
		catch (...)
		{
			cerr << "Error in receiving message from client 2."
				<< endl;
			break;
		}
		cout << "Receiving message from client 2: "
			<< cs->m_childConnection.getClient2IP() << ": " << msg
			<< endl;

		if (msg == "bye")
		{
			cout << "Received 'bye', closing connection to "
				<< "client 2: "
				<< cs->m_childConnection.getClient2IP()
				<< endl;
			// send bye to client 1
			try
			{
				cs->m_childConnection.sendToClient1("bye");
			}
			catch (...)
			{
				cerr << "Error in sending 'bye' to client 1"
					<< endl;
			}
			break;
		}
		else if (msg == "giveup")
		{
			cout << "Received 'giveup'" << endl;
			// send 'giveup' to second address only if no record
			// has been set previously for other client and break
			pthread_mutex_lock(clientRecordMutex);
			if (*client1Record == 'n')
			{
				*client2Record = 'g';
				pthread_mutex_unlock(clientRecordMutex);
				try
				{
					cs->m_childConnection.sendToClient1("giveup");
				}
				catch (...)
				{
					cerr << "Error in sending 'giveup' to "
						<< "client 1" << endl;
				}
			}
			break;
		}
		else
		{
			// forward message to client 1
			if (msg[0] == 'w')
			{
				pthread_mutex_lock(clientRecordMutex);
				if (*client1Record == 'n')
					*client2Record = 'w';
				pthread_mutex_unlock(clientRecordMutex);
			}
			else if (msg[0] == 't')
			{
				pthread_mutex_lock(clientRecordMutex);
				if (*client1Record == 'n')
					*client2Record = 't';
				pthread_mutex_unlock(clientRecordMutex);
			}
			try
			{
				cs->m_childConnection.sendToClient1(msg);
				cout << "Forwarding message from client 2: "
					<< cs->m_childConnection.getClient2IP()
					<< endl;
			}
			catch (...)
			{
				cerr << "Error forwarding message to client 1."
					<< endl;
				break;
			}
			if (msg[0] == 'w' || msg[0] == 't')
				break;
		}
	} while (FOREVER);

	return NULL;
}
