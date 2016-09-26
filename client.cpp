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

    // connect to parent server
    res = create_socket_server(SERVERPORT);
    if (res != 0)
    {
        cout << "Could not create socket to parent server.  Exiting" << endl;
        exit(1);
    }
    memset(m_rcv_buf, 0, MAXBUFLEN);
    handle_syn_ack(m_rcv_buf);

    // close connection to parent server
    close(m_sockfd);
    freeaddrinfo(m_servinfo);

    // connect to child server
    sleep(2);
    res = create_socket_server(m_rcv_buf);
    if (res != 0)
    {
        cout << "Could not create socket to child server.  Exiting" << endl;
        exit(1);
    }
    memset(m_rcv_buf, 0, MAXBUFLEN);
    handle_syn_ack(m_rcv_buf);

    if (strcmp(m_rcv_buf, "player-1") == 0)
    {
        cout << "You are player 1." << endl;
        m_is_p1 = true;
    }
    else if (strcmp(m_rcv_buf, "player-2") == 0)
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

        res = sendto(m_sockfd, "bye", 3, 0, m_p->ai_addr, m_p->ai_addrlen);
        if (res == -1)
            perror("client: sendto exiting");

        close(m_sockfd);
        freeaddrinfo(m_servinfo);
    }
}

int Client::create_socket_server(const char* port)
{
    struct addrinfo hints;
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

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
    int numbytes;
    char buf[MAXBUFLEN];
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    int got_ack;
    
    memset(buf, 0, MAXBUFLEN);
    addr_len = sizeof(their_addr);
    got_ack = 0;
    
    // send initial SYN and make sure receive ACK from server within certain
    // time.  if not receive ACK within 15 seconds, exit client.
    numbytes = sendto(m_sockfd, "SYN", 3, 0, m_p->ai_addr, m_p->ai_addrlen);
    if (numbytes == -1)
    {
        perror("client: SYN");
        exit(1);
    }
    cout << "Sent SYN to server, waiting for ACK..." << endl;
    pthread_t thread_timer;
    pthread_create(&thread_timer, NULL, &(Client::timer_countdown), &got_ack);

    numbytes = recvfrom(m_sockfd, buf, MAXBUFLEN-1, 0,
                        (struct sockaddr*)&their_addr, &addr_len);
    if (numbytes == -1)
        perror("recvfrom ACK");
    
    got_ack = 1;
    cout << "Received ACK from server." << endl;
    memcpy(resp, buf, MAXBUFLEN);
}

int Client::send_position(int pos)
{
    int numbytes;

    char buf[1];
    buf[0] = pos + '0';
    numbytes = sendto(m_sockfd, buf, 1, 0, m_p->ai_addr, m_p->ai_addrlen);
    if (numbytes == -1)
        return -1;
    return 0;
}

int Client::receive_position()
{
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;

    char buf[1];
    numbytes = recvfrom(m_sockfd, buf, 1, 0, (struct sockaddr*)&their_addr,
                        &addr_len);
    if (numbytes == -1)
        return -1;
    return (buf[0] - '0');
}
