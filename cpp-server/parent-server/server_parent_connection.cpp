#include "server_parent_connection.h"
#include <iostream>
#include <sstream>
#include "../utilities.h"
#include <string>
#include <sys/socket.h>  // for SOCK_STREAM, etc.
#include <netdb.h>  // for getaddrinfo(...), struct addrinfo, etc.
#include <cstring>
#include <sys/time.h>  // for struct timeval
#include "../constants.h"
#include "../exceptions.h"
#include <unistd.h>  // for FD_SET, select(...), etc.
#include <arpa/inet.h>  // for inet_ntop(...)
using namespace std;

ServerParentConnection::ServerParentConnection(int hostPort)
{
	m_hostPort = hostPort;

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;  // use my IP
	string hostPortStr = intToStr(hostPort);

	int st = getaddrinfo(NULL, hostPortStr.c_str(), &hints, &m_servinfo);
	if (st != 0)
	{
		cerr << "ServerParentConnection::ServerParentConnection::"
			<< "getaddrinfo: " << gai_strerror(st) << endl;
		throw ConnectionError;
	}

	// loop through all the results and bind to the first we can
	struct addrinfo* p;
	for (p = m_servinfo; p != NULL; p = p->ai_next)
	{
		if ((m_sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1)
			continue;
		int yes = 1;
		if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1)
		{
			cerr << "ServerParentConnection::"
				<< "ServerParentConnection::setsockopt"
				<< endl;
			throw ConnectionError;
		}
		if (bind(m_sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(m_sockfd);
			continue;
		}
		break;
	}
	if (p == NULL)
	{
		cerr << "Failed to bind socket." << endl;
		throw ConnectionError;
	}

	if ((listen(m_sockfd, BACKLOG)) == -1)
	{
		cerr << "ServerParentConnection::ServerParentConnection::"
			<< "listen" << endl;
		throw ConnectionError;
	}
}

ServerParentConnection::~ServerParentConnection()
{
	close(m_sockfd);
	freeaddrinfo(m_servinfo);
}

string ServerParentConnection::receiveFrom(int time)
{
	fd_set set;
	struct timeval timeout;
	FD_ZERO(&set);
	FD_SET(m_clientSockfd, &set);
	timeout.tv_sec = time;
	timeout.tv_usec = 0;

	int numbytes;
	int status;

	char buf[MAXBUFLEN];
	memset(buf, 0, MAXBUFLEN);

	status = select(m_clientSockfd + 1, &set, NULL, NULL, &timeout);
	if (status == -1)
		throw runtime_error("ServerParentConnection::receiveFrom::select");
	else if (status == 0)
		throw TimeoutError;  // timeout
	else
	{
		for (numbytes = 0; numbytes < MAXBUFLEN; numbytes += status)
		{
			status = recv(m_clientSockfd, buf + numbytes,
					MAXBUFLEN - numbytes, 0);
			if (status <= 0)
				break;
		}
		if (status == -1)
			throw runtime_error("ServerParentConnection::receiveFrom::read");
		else if (status == 0)
			throw DisconnectError;  // disconnect
		else
		{
			string toReturn(buf);  // read successful
			return toReturn;
		}
	}

}

void ServerParentConnection::sendTo(string text)
{
	int status;
	int numbytes;
	char buf[MAXBUFLEN];

	memset(buf, 0, MAXBUFLEN);
	strcpy(buf, text.c_str());

	for (numbytes = 0; numbytes < MAXBUFLEN; numbytes += status)
	{
		status = send(m_clientSockfd, buf + numbytes,
				MAXBUFLEN - numbytes, MSG_NOSIGNAL);
		if (status == -1)
			throw runtime_error("ServerParentConnection::sendTo");
	}

}

void ServerParentConnection::acceptClient(int time)
{
	if (time != 0)
	{
		fd_set set;
		struct timeval timeout;
		FD_ZERO(&set);
		FD_SET(m_sockfd, &set);
		timeout.tv_sec = time;
		timeout.tv_usec = 0;

		int rv = select(m_sockfd + 1, &set, NULL, NULL, &timeout);
		if (rv == -1)
		{
			string msg = "ServerParentConnection::acceptClient::select";
			throw runtime_error(msg);
		}
		else if (rv == 0)
			throw TimeoutError;  // timeout
	}

	int client_sockfd;
	struct sockaddr their_addr;
	socklen_t addr_len = sizeof(their_addr);

	client_sockfd = accept(m_sockfd, &their_addr, &addr_len);
	if (client_sockfd == -1)
		throw runtime_error("ServerParentConnection::acceptClient::accept");
	else
	{
		// accept successful
		struct sockaddr_in* their_addr_v4;
		their_addr_v4 = (struct sockaddr_in*)&their_addr;
		char dstIPStr[INET_ADDRSTRLEN];
		memset(dstIPStr, 0, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &their_addr_v4->sin_addr, dstIPStr,
				sizeof(dstIPStr));
		string temp(dstIPStr);
		m_clientIP = temp;
		m_clientPort = ntohs(their_addr_v4->sin_port);
		m_clientSockfd = client_sockfd;
	}
}

void ServerParentConnection::closeClient()
{
	close(m_clientSockfd);
}
