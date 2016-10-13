#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <map>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include "connection.h"
using namespace std;

#define LISTENPORT 4950  // the port clients will be connecting to
#define MAXBUFLEN 100

int setup_connection(int& sockfd, struct addrinfo* servinfo, int port_int)
{
    int status;
    struct addrinfo hints, *p;
    char port[MAXBUFLEN];
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // use my IP
    sprintf(port, "%d", port_int);

    if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
    {
        cerr << "getaddrinfo: " << gai_strerror(status) << endl;
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            continue;
        }
//        int yes = 1;
//        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
//                       sizeof(int)) == -1)
//        {
//            perror("setsockopt");
//            exit(1);
//        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        cerr << "Failed to bind socket" << endl;
        return 2;
    }

    if ((listen(sockfd, BACKLOG)) == -1)
    {
        perror("listen");
        return 2;
    }

    return 0;
}

void handle_syn_port(int sockfd, int& curr_port, int& client_port,
                     map<int, int>& ports_used, int& sockfd_client)
{
    int status;
    struct sockaddr their_addr;
    struct sockaddr_in* their_addr_v4;
    socklen_t addr_len;
    char buf[MAXBUFLEN];
    char s[INET_ADDRSTRLEN];
    char port[MAXBUFLEN];

    addr_len = sizeof(their_addr);
    memset(buf, 0, MAXBUFLEN);
    memset(s, 0, INET_ADDRSTRLEN);
    their_addr_v4 = NULL;
    
//    if ((listen(sockfd, BACKLOG)) == -1)
//    {
//        perror("listen");
//        return;
//    }

//    cout << "asdfkljsadlkfjsaldfj" << endl;
    sockfd_client = accept(sockfd, &their_addr, &addr_len);
    if (sockfd_client == -1)
    {
        perror("accept");
        return;
    }
    cout << "Accepted client." << endl;

    status = receive_from(sockfd_client, buf, MAXBUFLEN-1);
    if (status == -1)
    {
        perror("recvfrom SYN");
        return;
    }

    if (strcmp(buf, "SYN") != 0)
        return;

    their_addr_v4 = (struct sockaddr_in*)&their_addr;
    inet_ntop(AF_INET, &(their_addr_v4->sin_addr), s, sizeof(s));
    cout << "Got SYN from " << s << ", " << their_addr_v4->sin_port << endl;

    curr_port = LISTENPORT + 1;
    // priority should be to find count == 1 first
    for (; curr_port <= client_port; curr_port++)
    {
        if (ports_used[curr_port] == 1)
        {
            if (curr_port == client_port)
                client_port++;
            break;
        }
    }
    if (ports_used[curr_port] != 1)
    {
        curr_port = LISTENPORT + 1;
        for (; curr_port <= client_port; curr_port++)
        {
            if (ports_used[curr_port] == 0)
            {
                ports_used[curr_port] = 1;
                break;
            }
        }
    }
    else
        ports_used[curr_port] = 2;

    sprintf(port, "%d", curr_port);
    status = send_to_address(sockfd_client, port);
    if (status == -1)
        perror("sendto ACK");

    cout << "Sent ACK to use port: " << port << endl;
}

struct client_thread_params
{
    int* sockfd_curr_client;
    int* sockfd_other_client;
    struct sockaddr_in* addr_v4;
    int* exit_function;
};

void* client_thread(void* parameters)
{
    struct client_thread_params* params;
    params = (struct client_thread_params*)parameters;

    int status;
    char buf[MAXBUFLEN];
    char addr_str[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &(params->addr_v4->sin_addr),
              addr_str, sizeof(addr_str));
    
    while (*(params->exit_function) == 0)
    {
        memset(buf, 0, MAXBUFLEN);
        
        status = receive_from(*(params->sockfd_curr_client), buf, MAXBUFLEN-1);
        if (status == -1)
        {
            perror("recv");
        }
        
        if (strcmp(buf, "bye") == 0)
        {
            cout << "Received 'bye', closing connection to "
                 << addr_str << ", "
                 << params->addr_v4->sin_port
                 << endl;
            
            if (*(params->sockfd_other_client) != -1)
            {
                // send bye to second address
                status = send_to_address(*(params->sockfd_other_client), "bye");
                if (status == -1)
                {
                    perror("server: sendto");
                    exit(1);
                }
            }
            return NULL;
        }
        else if (strcmp(buf, "giveup") == 0)
        {
            cout << "Received 'giveup'" << endl;

            // send giveup to second address
            status = send_to_address(*(params->sockfd_other_client), "giveup");
            if (status == -1)
            {
                perror("server: sendto");
                exit(1);
            }
            return NULL;
        }
        else if (*(params->sockfd_other_client) == -1)
        {
        }
        else
        {
            // forward message from first_addr to second_addr
            status = send_to_address(*(params->sockfd_other_client), buf);
            if (status == -1)
            {
                perror("server: forward to second_addr");
                exit(1);
            }
        }
    }
    return NULL;
}

void handle_match_msg(int sockfd)
{
    int status;

    struct sockaddr_in *their_addr_v4 = NULL;
    
    struct sockaddr_in first_addr;
    struct sockaddr_in second_addr;
    memset(&first_addr, 0, sizeof(first_addr));
    memset(&second_addr, 0, sizeof(second_addr));

    struct sockaddr_in *first_addr_v4 = NULL;
    struct sockaddr_in *second_addr_v4 = NULL;

    char their_addr_str[INET_ADDRSTRLEN];
    char first_addr_str[INET_ADDRSTRLEN];
    char second_addr_str[INET_ADDRSTRLEN];

    struct sockaddr their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len = sizeof(their_addr);
    char s[INET_ADDRSTRLEN];

    int sockfd_client_1 = -1;
    int sockfd_client_2 = -1;

    int exit_function = 0;

    while (sockfd_client_1 == -1 && exit_function == 0)
    {
        sockfd_client_1 = accept(sockfd, &their_addr, &addr_len);
    }
    memcpy(&first_addr, &their_addr, sizeof(their_addr));
    first_addr_v4 = (struct sockaddr_in*)&first_addr;
    // send ACK to client 1 (player 1)
    status = send_to_address(sockfd_client_1, "player-1");
    if (status == -1)
    {
        perror("server: ACK to first_addr");
        exit(1);
    }
    cout << "Waiting for second client to connect..." << endl;
    struct client_thread_params first_thread_params;
    first_thread_params.sockfd_curr_client = &sockfd_client_1;
    first_thread_params.sockfd_other_client = &sockfd_client_2;
    first_thread_params.addr_v4 = first_addr_v4;
    first_thread_params.exit_function = &exit_function;
    pthread_t first_thread;
    pthread_create(&first_thread, NULL, &client_thread, &first_thread_params);

    while (sockfd_client_2 == -1 && exit_function == 0)
    {
        sockfd_client_2 = accept(sockfd, &their_addr, &addr_len);
    }
    memcpy(&second_addr, &their_addr, sizeof(their_addr));
    second_addr_v4 = (struct sockaddr_in*)&second_addr;
    // send ACK to client 2 (player 2)
    status = send_to_address(sockfd_client_2, "player-2");
    if (status == -1)
    {
        perror("server: ACK to second_addr");
        exit(1);
    }
    status = send_to_address(sockfd_client_1, "player-2");
    if (status == -1)
    {
        perror("server: ACK to first_addr");
        exit(1);
    }
    cout << "Second client connected." << endl;
    struct client_thread_params second_thread_params;
    second_thread_params.sockfd_curr_client = &sockfd_client_2;
    second_thread_params.sockfd_other_client = &sockfd_client_1;
    second_thread_params.addr_v4 = second_addr_v4;
    second_thread_params.exit_function = &exit_function;
    pthread_t second_thread;
    pthread_create(&second_thread, NULL, &client_thread, &second_thread_params);

    pthread_join(first_thread, NULL);
    pthread_join(second_thread, NULL);
}

int receive_from(int sockfd, char* buf, size_t size)
{
    int numbytes;
    numbytes = recv(sockfd, buf, size, 0);

    if (numbytes == -1)
        return -1;
    return 0;
}

int send_to_address(int sockfd, const char* text)
{
    int numbytes;
    numbytes = send(sockfd, text, strlen(text), 0);
    
    if (numbytes == -1)
        return -1;
    return 0;
}
