#include "client.h"
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <pthread.h>
#include <ctime>
#include <cstdlib>
#include <netinet/in.h>
#include <unistd.h>
using namespace std;

Client::Client()
{
    m_p = NULL;
    m_servinfo = NULL;

    int res;
    char buf[MAXBUFLEN];

    // connect to parent server
    res = create_socket_server(SERVERPORT);
    if (res != 0)
    {
        cout << "Could not create socket to parent server.  Exiting" << endl;
        exit(1);
    }
    memset(buf, 0, MAXBUFLEN);
    get_num_ppl();
    handle_syn_ack(buf);

    // close connection to parent server
    close(m_sockfd);
    freeaddrinfo(m_servinfo);

    // connect to child server
    sleep(1);  // wait for child server to establish before create socket
    res = create_socket_server(buf);
    if (res != 0)
    {
        cout << "Could not create socket to child server.  Exiting" << endl;
        exit(1);
    }
    memset(buf, 0, MAXBUFLEN);
    cout << "connected to child server" << endl;
    handle_syn_ack(buf);

    // get assigned player-1 or player-2
    if (strcmp(buf, "player-1") == 0)
    {
        cout << "You are player 1." << endl;
        m_is_p1 = true;
        cout << "Waiting for player 2 to connect..." << endl;

        do
        {
            res = receive_from_server(buf);
            if (res == -1)
            {
                perror("recvfrom ACK");
                exit(1);
            }
        } while (strcmp(buf, "player-2") != 0);

        cout << "Player 2 has connected.  Starting game." << endl;

    }
    else if (strcmp(buf, "player-2") == 0)
    {
        cout << "You are player 2." << endl;
        m_is_p1 = false;
    }
    else
    {
        cout << "Try connecting again." << endl;
        exit(0);
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

void* Client::timer_countdown(void* parameters)
{
    time_t start;
    time_t end;
    struct timer_params* params = (struct timer_params*)parameters;

    time(&start);
    do
    {
        time(&end);
        if (*(params->got_ack))
            return NULL;
    } while(difftime(end, start) < params->seconds);
    cout << "Connection failed.  Try again." << endl;
    exit(0);
    return NULL;
}

void Client::handle_syn_ack(char resp[MAXBUFLEN])
{
    int res;
    char buf[MAXBUFLEN];
    pthread_t timer_thread;
    int got_ack;

    struct timer_params params;
    params.seconds = 15;
    params.got_ack = &got_ack;

    memset(buf, 0, MAXBUFLEN);

    got_ack = 0;
    pthread_create(&timer_thread, NULL, &(Client::timer_countdown), &params);
    res = receive_from_server(buf);  // should possibly be aborted by timer
    if (res == -1)
    {
        perror("recvfrom ACK");
        exit(1);
    }
    got_ack = 1;
    pthread_join(timer_thread, NULL);

    cout << "Received ACK from server." << endl;
    memcpy(resp, buf, MAXBUFLEN);
}

void Client::get_num_ppl()
{
    int res;
    char buf[MAXBUFLEN];
    pthread_t timer_thread;
    int got_ack;

    struct timer_params params;
    params.seconds = 15;
    params.got_ack = &got_ack;

    memset(buf, 0, MAXBUFLEN);

    got_ack = 0;
    pthread_create(&timer_thread, NULL, &(Client::timer_countdown), &params);
    res = receive_from_server(buf);  // should possibly be aborted by timer
    if (res == -1)
    {
        perror("recvfrom num_ppl");
        exit(1);
    }
    got_ack = 1;
    pthread_join(timer_thread, NULL);

    cout << "Number of people online: " << buf << endl;
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

int Client::send_to_server(const char* text)
{
    int numbytes;
    numbytes = send(m_sockfd, text, strlen(text), 0);
    if (numbytes == -1)
        return -1;
    return 0;
}

int Client::receive_from_server(char* buf)
{
    int numbytes;
    numbytes = recv(m_sockfd, buf, MAXBUFLEN, 0);
    return numbytes;
}
