#include "child_server.h"
#include "write_named_pipe.h"
#include "utilities.h"
#include "constants.h"
#include "db-accessor.h"
#include "exceptions.h"
#include <string>
#include <iostream>
#include <cstring>
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
	string password;
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
		m_client1LoginProvided = isLoginProvided(login,
				m_client1Username, password);
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

	// if login provided, get win/loss records
	setClientWinLossMsg(m_client1WinLossMsg, m_client1Username,
			m_client1LoginProvided);

	// start thread to wait for client 2 to login
}

bool ChildServer::isLoginProvided(const string& login, string& username,
		string& password)
{
	if (login[0] == ',')
		return false;

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

	username = username_c;
	password = password_c;
	return true;
}

void ChildServer::setClientWinLossMsg(string& clientWinLossMsg,
		const string& username, bool loginProvided)
{
	if (!loginProvided)
	{
		clientWinLossMsg = "r,,";
		return;
	}

	char win_c[MAXBUFLEN];
	char loss_c[MAXBUFLEN];
	string win;
	string loss;

	memset(win_c, 0, MAXBUFLEN);
	memset(loss_c, 0, MAXBUFLEN);
	get_win_loss_record(username.c_str(), win_c, loss_c);
	win = win_c;
	loss = loss_c;

	clientWinLossMsg = "r" + win + "," + loss + ",";
}

ChildServer::~ChildServer()
{
	WriteNamedPipe writeNamedPipe(false);
	string portStr = intToStr(m_port);
	writeNamedPipe.writePipe(portStr, portStr.length());
}
