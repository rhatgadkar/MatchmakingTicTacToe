#include "client.h"
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <string>
using namespace std;

Client::Client(string username, string password)
{
	m_p = NULL;
	m_servinfo = NULL;
	m_username = username;
	m_password = password;

	int retries;
	for (retries = 0; retries < 10; retries++)
	{
		int res;
		char buf[MAXBUFLEN];

		close(m_sockfd);

		// connect to parent server
		res = create_socket_server(SERVERPORT);
		if (res != 0)
		{
			cerr << "Could not create socket to parent server.  Exiting" << endl;
			continue;
		}
		memset(buf, 0, MAXBUFLEN);
		res = get_num_ppl();
		if (res == -1)
			continue;
		else if (res == 2)
		{
			cout << "Child servers are full. Retrying." << endl;
			retries = 0;
			sleep(15);
			continue;
		}
		if (!handle_syn_ack(buf))
			continue;

		// close connection to parent server
		close(m_sockfd);
		freeaddrinfo(m_servinfo);

		// connect to child server
		sleep(1);  // wait for child server to establish before create socket
		res = create_socket_server(buf);
		if (res != 0)
		{
			cerr << "Could not create socket to child server.  Exiting" << endl;
			continue;
		}
		memset(buf, 0, MAXBUFLEN);
		cout << "connected to child server" << endl;
		if (!handle_child_syn_ack(buf))
		{
			// possible collision to a child server
			retries = 0;
			continue;
		}

		// get assigned player-1 or player-2
		if (strcmp(buf, "player-1") == 0)
		{
			cout << "You are player 1." << endl;
			m_is_p1 = true;
			cout << "Waiting for player 2 to connect..." << endl;

			bool invalidres = false;
			do
			{
				memset(buf, 0, MAXBUFLEN);
				res = receive_from_server(buf);
				if (res == -1)
				{
					perror("recvfrom ACK");
					invalidres = true;
					break;
				}
			} while (buf[0] != 'r' && buf[0] != ',');
			if (invalidres)
				continue;

			if (buf[0] == 'r')
				Record = buf + 1;
			cout << "Player 2 has connected.  Starting game." << endl;
			break;
		}
		else if (buf[0] == 'r' || buf[0] == ',')
		{
			if (buf[0] == 'r')
				Record = buf + 1;
			cout << "You are player 2." << endl;
			m_is_p1 = false;
			break;
		}
		else if (strcmp(buf, "invalidl") == 0)
		{
			cout << "Invalid login credentials. Exiting." << endl;
			exit(0);
		}
		else
		{
			cout << "User is currently in game." << endl;
			exit(0);
		}
	}
	if (retries == 10)
	{
		cout << "Connection to server failed." << endl;
		exit(1);
	}
}

Client::~Client()
{
	if (m_p != NULL)
	{
		cout << "Client is exiting.  Closing server." << endl;

		if (!send_bye())
		{
			perror("client: sendto exiting");
			exit(1);
		}

		freeaddrinfo(m_servinfo);
	}
}

int Client::create_socket_server(const char* port)
{
	struct addrinfo hints;
	int rv;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	rv = getaddrinfo(SERVERIP, port, &hints, &m_servinfo);
	if (rv != 0)
	{
		cerr << "getaddrinfo: " << gai_strerror(rv) << endl;
		return 1;
	}

	// loop through all the results and make a socket
	for (m_p = m_servinfo; m_p != NULL; m_p = m_p->ai_next)
	{
		m_sockfd = socket(m_p->ai_family, m_p->ai_socktype, m_p->ai_protocol);
		if (m_sockfd == -1)
		{
			perror("client: socket");
			continue;
		}

		if ((connect(m_sockfd, m_p->ai_addr, m_p->ai_addrlen)) == -1)
		{
			close(m_sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (m_p == NULL)
	{
		cerr << "client: failed to create socket" << endl;
		return 2;
	}

	return 0;
}

bool Client::handle_syn_ack(char resp[MAXBUFLEN])
{
	int res;
	char buf[MAXBUFLEN];

	memset(buf, 0, MAXBUFLEN);

	res = receive_from(buf, 15);
	if (res <= 0)
	{
		perror("recvfrom ACK");
		return false;
	}

	cout << "Received ACK from server." << endl;
	memcpy(resp, buf, MAXBUFLEN);
	return true;
}

bool Client::handle_child_syn_ack(char resp[MAXBUFLEN])
{
	int res;
	char buf[MAXBUFLEN];

	memset(buf, 0, MAXBUFLEN);

	string login = m_username + "," + m_password;
	res = send_to_server(login.c_str());
	if (res < 0)
	{
		cerr << "Send login failed." << endl;
		return false;
	}

	res = receive_from(buf, 15);
	if (res <= 0)
	{
		perror("recvfrom ACK");
		return false;
	}

	cout << "Received ACK from server." << endl;
	memcpy(resp, buf, MAXBUFLEN);
	return true;
}

int Client::get_num_ppl()
{
	int res;
	char buf[MAXBUFLEN];

	memset(buf, 0, MAXBUFLEN);

	res = receive_from(buf, 15);
	if (res <= 0)
	{
		perror("get_num_ppl");
		return -1;
	}

	if (buf[0] == 'b')
		return 2;
	cout << "Number of people online: " << buf << endl;
	return 1;
}

bool Client::send_position(int pos)
{
	int res;
	char buf[MAXBUFLEN];

	memset(buf, 0, MAXBUFLEN);
	buf[0] = pos + '0';

	res = send_to_server(buf);
	if (res == -1)
		return false;
	return true;
}

bool Client::send_giveup()
{
	int res;

	res = send_to_server("giveup");
	if (res == -1)
		return false;
	return true;
}

bool Client::send_bye()
{
	int res;

	res = send_to_server("bye");
	if (res == -1)
		return false;
	return true;
}

bool Client::send_win(int pos)
{
	int res;
	char buf[MAXBUFLEN];

	memset(buf, 0, MAXBUFLEN);
	buf[0] = 'w';
	buf[1] = pos + '0';

	res = send_to_server(buf);
	if (res == -1)
		return false;
	return true;
}

bool Client::send_tie(int pos)
{
	int res;
	char buf[MAXBUFLEN];

	memset(buf, 0, MAXBUFLEN);
	buf[0] = 't';
	buf[1] = pos + '0';

	res = send_to_server(buf);
	if (res == -1)
		return false;
	return true;
}

int Client::send_to_server(const char* text)
{
	int status;
	int numbytes;
	char buf[MAXBUFLEN];

	memset(buf, 0, MAXBUFLEN);
	strcpy(buf, text);

	for (numbytes = 0; numbytes < MAXBUFLEN; numbytes += status)
	{
		status = send(m_sockfd, text + numbytes, MAXBUFLEN - numbytes, 0);
		if (status == -1)
			return -1;
	}
	return 0;
}

int Client::receive_from(char* buf, int time)
{
	fd_set set;
	struct timeval timeout;
	FD_ZERO(&set);
	FD_SET(m_sockfd, &set);
	timeout.tv_sec = time;
	timeout.tv_usec = 0;

	int status;
	int numbytes;

	status = select(m_sockfd + 1, &set, NULL, NULL, &timeout);
	if (status == -1)
	{
		perror("select");
		return -1;
	}
	else if (status == 0)
		return -2;  // timeout
	else
	{
		for (numbytes = 0; numbytes < MAXBUFLEN; numbytes += status)
		{
			status = recv(m_sockfd, buf + numbytes, MAXBUFLEN - numbytes, 0);
			if (status <= 0)
				break;
		}
		if (status == -1)
		{
			perror("read");
			return -1;
		}
		else if (status == 0)
			return 0;  // disconnect
		else
		{
			return numbytes;  // read successful
		}
	}
}

int Client::receive_from_server(char* buf)
{
	int status;
	int numbytes;

	for (numbytes = 0; numbytes < MAXBUFLEN; numbytes += status)
	{
		status = recv(m_sockfd, buf + numbytes, MAXBUFLEN - numbytes, 0);
		if (status <= 0)
			return status;
	}
	return numbytes;
}
