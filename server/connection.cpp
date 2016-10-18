#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include "connection.h"
using namespace std;

#define LISTENPORT 4950  // the port clients will be connecting to
#define MAXBUFLEN 100

static int receive_from(int sockfd, char* buf, size_t size);
static int send_to_address(int sockfd, const char* text);

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
        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }
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
                     int* shm_ports_used, int& sockfd_client)
{
    int status;
    struct sockaddr their_addr;
    struct sockaddr_in* their_addr_v4;
    socklen_t addr_len;
    char buf[MAXBUFLEN];
    char s[INET_ADDRSTRLEN];
    char port[MAXBUFLEN];
    int* shm_iter;

    addr_len = sizeof(their_addr);
    memset(buf, 0, MAXBUFLEN);
    memset(s, 0, INET_ADDRSTRLEN);
    their_addr_v4 = NULL;
    
    sockfd_client = accept(sockfd, &their_addr, &addr_len);
    if (sockfd_client == -1)
    {
        perror("accept");
        return;
    }
    cout << "Accepted client." << endl;

    their_addr_v4 = (struct sockaddr_in*)&their_addr;
    inet_ntop(AF_INET, &(their_addr_v4->sin_addr), s, sizeof(s));
    cout << "client: " << their_addr_v4->sin_port 
         << " connected to parent server." << endl;

    curr_port = LISTENPORT + 1;
    port_to_shm_iter(curr_port, &shm_iter, shm_ports_used);
    // priority should be to find count == 1 first
    for (; curr_port <= client_port; curr_port++)
    {
        port_to_shm_iter(curr_port, &shm_iter, shm_ports_used);
        if (*shm_iter == 1)
        {
            if (curr_port == client_port)
                client_port++;
            break;
        }
    }
    if (*shm_iter != 1)
    {
        curr_port = LISTENPORT + 1;
        for (; curr_port <= client_port; curr_port++)
        {
            port_to_shm_iter(curr_port, &shm_iter, shm_ports_used);
            if (*shm_iter == 0)
                break;
        }
    }

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
    int sockfd;
    pthread_t other_id;
    int* shm_iter;
};

void* client_thread(void* parameters)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    struct client_thread_params* params;
    params = (struct client_thread_params*)parameters;

    int status;
    char buf[MAXBUFLEN];
    char addr_str[INET_ADDRSTRLEN];

    if (*(params->sockfd_curr_client) == -1 && *(params->shm_iter) == 1)
    {
        struct sockaddr their_addr;
        socklen_t addr_len = sizeof(their_addr);

        cout << "Waiting for second client to connect..." << endl;
        *(params->sockfd_curr_client) = accept(params->sockfd, &their_addr,
                                               &addr_len);
        *(params->shm_iter) = *(params->shm_iter) + 1;

        params->addr_v4 = (struct sockaddr_in*)&their_addr;
        // send ACK to client 2 (player 2)
        status = send_to_address(*(params->sockfd_curr_client), "player-2");
        if (status == -1)
        {
            perror("server: ACK to second_addr");
            exit(1);
        }
        status = send_to_address(*(params->sockfd_other_client), "player-2");
        if (status == -1)
        {
            perror("server: ACK to first_addr");
            exit(1);
        }
        cout << "Second client connected: " << params->addr_v4->sin_port
             << endl;
    }

    inet_ntop(AF_INET, &(params->addr_v4->sin_addr),
              addr_str, sizeof(addr_str));

    for (;;)
    {
        memset(buf, 0, MAXBUFLEN);
        
        status = receive_from(*(params->sockfd_curr_client), buf, MAXBUFLEN-1);
        cout << "receiving message from " << params->addr_v4->sin_port << ": "
             << buf << endl;
        if (status == -1)
        {
            perror("recv");
        }
        if (status == 0)
        {
            if (*(params->sockfd_other_client) == -1)
            {
                cout << "Received 'bye', closing connection to "
                     << addr_str << ", "
                     << params->addr_v4->sin_port
                     << endl;
                pthread_cancel(params->other_id);
                return NULL;
            }
            else
            {
                cout << "Received 'giveup'" << endl;

                // send giveup to second address
                status = send_to_address(*(params->sockfd_other_client),
                                         "giveup");
                if (status == -1)
                {
                    perror("server: sendto");
                    exit(1);
                }
                pthread_cancel(params->other_id);
                return NULL;
            }
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
                status = send_to_address(*(params->sockfd_other_client),
                                         "bye");
                if (status == -1)
                {
                    perror("server: sendto");
                    exit(1);
                }
            }
            pthread_cancel(params->other_id);
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
            pthread_cancel(params->other_id);
            return NULL;
        }
        else if (*(params->sockfd_other_client) == -1)
        {
        }
        else
        {
            // forward message from first_addr to second_addr
            status = send_to_address(*(params->sockfd_other_client), buf);
            cout << "forwarding message from " << params->addr_v4->sin_port
                 << ": " << buf << endl;
            if (status == -1)
            {
                perror("server: forward to second_addr");
                exit(1);
            }
        }
    }
    pthread_cancel(params->other_id);
    return NULL;
}

void handle_match_msg(int sockfd, int* shm_iter)
{
    int status;

    struct sockaddr their_addr;
    socklen_t addr_len = sizeof(their_addr);

    int sockfd_client_1 = -1;
    int sockfd_client_2 = -1;

    if (sockfd_client_1 == -1 && *shm_iter == 0)
    {
        sockfd_client_1 = accept(sockfd, &their_addr, &addr_len);
        (*shm_iter)++;
    }
    // send ACK to client 1 (player 1)
    status = send_to_address(sockfd_client_1, "player-1");
    if (status == -1)
    {
        perror("server: ACK to first_addr");
        exit(1);
    }

    pthread_t first_thread;
    pthread_t second_thread;

    struct client_thread_params first_thread_params;
    first_thread_params.sockfd_curr_client = &sockfd_client_1;
    first_thread_params.sockfd_other_client = &sockfd_client_2;
    first_thread_params.addr_v4 = (struct sockaddr_in*)&their_addr;
    first_thread_params.sockfd = sockfd;
    first_thread_params.other_id = second_thread;
    first_thread_params.shm_iter = shm_iter;
    cout << "First client connected: "
         << first_thread_params.addr_v4->sin_port << endl;
    pthread_create(&first_thread, NULL, &client_thread, &first_thread_params);

    struct client_thread_params second_thread_params;
    second_thread_params.sockfd_curr_client = &sockfd_client_2;
    second_thread_params.sockfd_other_client = &sockfd_client_1;
    second_thread_params.addr_v4 = NULL;
    second_thread_params.sockfd = sockfd;
    second_thread_params.other_id = first_thread;
    second_thread_params.shm_iter = shm_iter;
    pthread_create(&second_thread, NULL, &client_thread,
                   &second_thread_params);

    void* st;
    pthread_join(first_thread, &st);
    pthread_cancel(second_thread);
    if (st == PTHREAD_CANCELED)
        cout << "Thread 1 was canceled" << endl;
    else
        cout << "Thread 1 was not canceled" << endl;
    pthread_join(second_thread, &st);
    if (st == PTHREAD_CANCELED)
        cout << "Thread 2 was canceled" << endl;
    else
        cout << "Thread 2 was not canceled" << endl;

    close(sockfd_client_1);
    close(sockfd_client_2);
}

int receive_from(int sockfd, char* buf, size_t size)
{
    int numbytes;
    numbytes = recv(sockfd, buf, size, 0);
    if (numbytes == -1)
        return -1;
    return numbytes;
}

int send_to_address(int sockfd, const char* text)
{
    int numbytes;
    numbytes = send(sockfd, text, strlen(text), 0);
    if (numbytes == -1)
        return -1;
    return 0;
}

void port_to_shm_iter(int port, int** shm_iter, int* shm_ports_used)
{
    *shm_iter = shm_ports_used;
    for (size_t k = 0; k < port - LISTENPORT; k++)
        (*shm_iter)++;
}
