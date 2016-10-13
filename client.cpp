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
#include <signal.h>
using namespace std;

sig_atomic_t Client::sigint_check = 0;

void Client::sigint_handler(int s)
{
    Client::sigint_check = 1;
    cout << "got SIGINT" << endl;
}

void Client::sigint_ignore_handler(int s)
{
    cout << "got SIGINT" << endl;
}

void* Client::check_sigint(void* parameters)
{
    Client* c = (Client*)parameters;

    for (;;)
    {
        if (Client::sigint_check)
        {
            c->send_bye();
            cout << "You have quit searching" << endl;
            exit(0);
        }
    }
}

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
    handle_syn_ack(buf);

    // close connection to parent server
    close(m_sockfd);
    freeaddrinfo(m_servinfo);

    // connect to child server
    sleep(2);
    res = create_socket_server(buf);
    if (res != 0)
    {
        cout << "Could not create socket to child server.  Exiting" << endl;
        exit(1);
    }
    memset(buf, 0, MAXBUFLEN);
    handle_syn_ack(buf);

    // get assigned player-1 or player-2
    if (strcmp(buf, "player-1") == 0)
    {
        cout << "You are player 1." << endl;
        m_is_p1 = true;
        cout << "Waiting for player 2 to connect..." << endl;

        // check if SIGINT for quit searching
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = &(Client::sigint_handler);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGINT, &sa, NULL) == -1)
        {
            perror("sigaction");
            exit(1);
        }
        pthread_t thread_sigint_id;
        pthread_create(&thread_sigint_id, NULL, &(Client::check_sigint), this);

        do
        {
            res = receive_from_server(buf, MAXBUFLEN);
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
        cout << "hmm.  error." << endl;
}

Client::~Client()
{
    int res;

    if (m_p != NULL)
    {
        cout << "Client is exiting.  Closing server." << endl;

        if (!send_bye())
        {
            perror("client: sendto exiting");
            exit(1);
        }

        close(m_sockfd);
        freeaddrinfo(m_servinfo);
    }
}

int Client::create_socket_server(const char* port)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &(Client::sigint_ignore_handler);
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

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
    int* got_ack = (int*)parameters;

    time(&start);
    do
    {
        time(&end);
        if (*got_ack)
            return NULL;
    } while(difftime(end, start) < 15);
    cout << "Did not receive ACK in under 15 seconds.  Exiting." << endl;
    exit(0);
    return NULL;
}

void Client::handle_syn_ack(char resp[MAXBUFLEN])
{
//    struct sigaction sa;
//    memset(&sa, 0, sizeof(sa));
//    sa.sa_handler = &(Client::sigint_ignore_handler);
//    if (sigaction(SIGINT, &sa, NULL) == -1)
//    {
//        perror("sigaction");
//        exit(1);
//    }

    int res;
    char buf[MAXBUFLEN];
    int got_ack;
    
    memset(buf, 0, MAXBUFLEN);
    got_ack = 0;
    
    // send initial SYN and make sure receive ACK from server within certain
    // time.  if not receive ACK within 15 seconds, exit client.
    res = send_to_server("SYN");
    if (res == -1)
    {
        perror("client: SYN");
        exit(1);
    }
    cout << "Sent SYN to server, waiting for ACK..." << endl;
    pthread_t thread_timer;
    pthread_create(&thread_timer, NULL, &(Client::timer_countdown), &got_ack);

    res = receive_from_server(buf, MAXBUFLEN);
    if (res == -1)
    {
        perror("recvfrom ACK");
        exit(1);
    }
    
    got_ack = 1;
    cout << "Received ACK from server." << endl;
    memcpy(resp, buf, MAXBUFLEN);
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

int Client::receive_position()
{
    int res;
    char buf[MAXBUFLEN];

    memset(buf, 0, MAXBUFLEN);

    res = receive_from_server(buf, MAXBUFLEN);
    if (res == -1)
        return -1;
    return (buf[0] - '0');
}

bool Client::receive_giveup()
{
    int res;
    char buf[MAXBUFLEN];

    memset(buf, 0, MAXBUFLEN);

    res = receive_from_server(buf, MAXBUFLEN);
    if (res == -1 || strcmp(buf, "giveup") != 0)
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

int Client::receive_from_server(char* buf, size_t size)
{
    int numbytes;
    numbytes = recv(m_sockfd, buf, size, 0);
    if (numbytes == -1)
        return -1;
    return 0;
}
