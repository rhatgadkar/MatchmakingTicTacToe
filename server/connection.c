#include <sys/types.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/select.h>
#include <sys/time.h>
#include "connection.h"

#define LISTENPORT 4950  // the port clients will be connecting to
#define MAXBUFLEN 100

static int receive_from(int sockfd, char* buf, int time);
static int send_to_address(int sockfd, const char* text);
static int accept_timer(int sockfd, struct sockaddr* their_addr,
                        socklen_t* addr_len, int time);

int setup_connection(int* sockfd, struct addrinfo* servinfo, int port_int)
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
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((*sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            continue;
        }
        int yes = 1;
        if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }
        if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(*sockfd);
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "Failed to bind socket.\n");
        return 2;
    }

    if ((listen(*sockfd, BACKLOG)) == -1)
    {
        perror("listen");
        return 2;
    }

    return 0;
}

int handle_syn_port(int sockfd, int* curr_port, int* client_port,
                     int* shm_ports_used, int* sockfd_client)
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
    
    *sockfd_client = accept(sockfd, &their_addr, &addr_len);
    if (*sockfd_client == -1)
    {
        perror("accept");
        return -1;
    }
    printf("Accepted client.\n");

    their_addr_v4 = (struct sockaddr_in*)&their_addr;
    inet_ntop(AF_INET, &(their_addr_v4->sin_addr), s, sizeof(s));
    printf("client: %s:%hu connected to parent server.\n", s,
           their_addr_v4->sin_port);

    // send number of people online to client
    int num_ppl = 0;
    char num_ppl_str[MAXBUFLEN];
    shm_iter = shm_ports_used;
    int k;
    for (k = 0; k < 1000; k++)
        num_ppl += (*shm_iter++);
    sprintf(num_ppl_str, "%d", num_ppl);
    status = send_to_address(*sockfd_client, num_ppl_str);
    if (status == -1)
    {
        perror("sendto num_ppl");
        return -1;
    }

    sleep(1);

    int old_client_port = *client_port;
    *curr_port = LISTENPORT + 1;
    port_to_shm_iter(*curr_port, &shm_iter, shm_ports_used);
    // priority should be to find count == 1 first
    for (; *curr_port <= *client_port; (*curr_port)++)
    {
        port_to_shm_iter(*curr_port, &shm_iter, shm_ports_used);
        if (*shm_iter == 1)
        {
            if (*curr_port == *client_port)
                (*client_port)++;
            break;
        }
    }
    if (*shm_iter != 1)
    {
        *curr_port = LISTENPORT + 1;
        for (; *curr_port <= *client_port; (*curr_port)++)
        {
            port_to_shm_iter(*curr_port, &shm_iter, shm_ports_used);
            if (*shm_iter == 0)
                break;
        }
    }

    sprintf(port, "%d", *curr_port);
    status = send_to_address(*sockfd_client, port);
    if (status == -1)
    {
        perror("sendto ACK");
        *client_port = old_client_port;
        return -1;
    }

    printf("Sent ACK to use port: %s\n", port);
    return 0;
}

struct client_thread_params
{
    int* sockfd_curr_client;
    int* sockfd_other_client;
    struct sockaddr_in* addr_v4;
    int sockfd;
    pthread_t other_id;
    int* shm_iter;
	int thread_canceled;
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

        printf("Waiting for second client to connect...\n");
        while (*(params->sockfd_curr_client) == -1)
            *(params->sockfd_curr_client) = accept(params->sockfd, &their_addr,
                                                   &addr_len);
        *(params->shm_iter) = *(params->shm_iter) + 1;

        params->addr_v4 = (struct sockaddr_in*)&their_addr;
        // send ACK to client 2 (player 2)
        status = send_to_address(*(params->sockfd_curr_client), "player-2");
        if (status == -1)
        {
            perror("server: ACK to second_addr");
            pthread_cancel(params->other_id);
            return NULL;
        }
        status = send_to_address(*(params->sockfd_other_client), "player-2");
        if (status == -1)
        {
            perror("server: ACK to first_addr");
            pthread_cancel(params->other_id);
            return NULL;
        }
        inet_ntop(AF_INET, &(params->addr_v4->sin_addr),
                  addr_str, sizeof(addr_str));
        printf("Second client connected: %s:%hu\n", addr_str,
               params->addr_v4->sin_port);
    }
    else
    {
        inet_ntop(AF_INET, &(params->addr_v4->sin_addr),
                  addr_str, sizeof(addr_str));
        printf("First client connected: %s:%hu\n", addr_str,
               params->addr_v4->sin_port);
    }


    for (;;)
    {
        memset(buf, 0, MAXBUFLEN);
        
        status = receive_from(*(params->sockfd_curr_client), buf, 120);
        printf("Receiving message from %s:%hu: %s\n", addr_str,
               params->addr_v4->sin_port, buf);
        if (status == -1)
        {
            perror("recv");
			break;
        }
        if (status == -2)
        {
            printf("Not receiving anything. Closing child server.\n");
            // send bye to second address
            status = send_to_address(*params->sockfd_other_client, "bye");
            if (status == -1)
                perror("server: sendto");
            break;
        }
        if (status == 0)
        {
            if (*(params->sockfd_other_client) == -1)
            {
                printf("Received 'bye', closing connection to %s:%hu\n",
                       addr_str, params->addr_v4->sin_port);
                break;
            }
            else
            {
                printf("Received 'giveup'\n");

                // send giveup to second address
                status = send_to_address(*(params->sockfd_other_client),
                                         "giveup");
                if (status == -1)
                    perror("server: sendto");
                break;
            }
        }
        
        if (strcmp(buf, "bye") == 0)
        {
            printf("Received 'bye', closing connection to %s:%hu\n",
                   addr_str, params->addr_v4->sin_port);
            
            if (*(params->sockfd_other_client) != -1)
            {
                // send bye to second address
                status = send_to_address(*(params->sockfd_other_client),
                                         "bye");
                if (status == -1)
                    perror("server: sendto");
            }
            break;
        }
        else if (strcmp(buf, "giveup") == 0)
        {
            printf("Received 'giveup'\n");

            // send giveup to second address
            status = send_to_address(*(params->sockfd_other_client), "giveup");
            if (status == -1)
                perror("server: sendto");
            break;
        }
        else if (*(params->sockfd_other_client) == -1)
        {
        }
        else
        {
            // forward message from first_addr to second_addr
            status = send_to_address(*(params->sockfd_other_client), buf);
            printf("Forwarding message from %s:%hu: %s\n", addr_str,
                   params->addr_v4->sin_port, buf);
            if (status == -1)
            {
                perror("server: forward to second_addr");
                break;
            }
        }
    }
	params->thread_canceled = 1;
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
        sockfd_client_1 = accept_timer(sockfd, &their_addr, &addr_len, 30);
        if (sockfd_client_1 == 1)
        {
            printf("Closing child server.\n");
            return;
        }
        (*shm_iter)++;
    }
    // send ACK to client 1 (player 1)
    status = send_to_address(sockfd_client_1, "player-1");
    if (status == -1)
    {
        perror("server: ACK to first_addr");
        return;
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
	first_thread_params.thread_canceled = 0;
    struct client_thread_params second_thread_params;
    second_thread_params.sockfd_curr_client = &sockfd_client_2;
    second_thread_params.sockfd_other_client = &sockfd_client_1;
    second_thread_params.addr_v4 = NULL;
    second_thread_params.sockfd = sockfd;
    second_thread_params.other_id = first_thread;
    second_thread_params.shm_iter = shm_iter;
	second_thread_params.thread_canceled = 0;

    pthread_create(&first_thread, NULL, &client_thread, &first_thread_params);

    pthread_create(&second_thread, NULL, &client_thread,
                   &second_thread_params);

	while (second_thread_params.thread_canceled == 0 &&
		   first_thread_params.thread_canceled == 0)
		;
	if (second_thread_params.thread_canceled)
	{
		pthread_join(second_thread, NULL);
		pthread_cancel(first_thread);
		pthread_join(first_thread, NULL);
	}
	else
	{
		pthread_join(first_thread, NULL);
		pthread_cancel(second_thread);
		pthread_join(second_thread, NULL);
	}

    close(sockfd_client_1);
    close(sockfd_client_2);
}

int accept_timer(int sockfd, struct sockaddr* their_addr, socklen_t* addr_len,
                 int time)
{
    fd_set set;
	struct timeval timeout;
	FD_ZERO(&set);
	FD_SET(sockfd, &set);
	timeout.tv_sec = time;
	timeout.tv_usec = 0;
	
	int client_sockfd;
	int rv;

	rv = select(sockfd + 1, &set, NULL, NULL, &timeout);
	if (rv == -1)
	{
		perror("select");
		return -1;
	}
	else if (rv == 0)
		return 1;  // timeout
	else
	{
		client_sockfd = accept(sockfd, their_addr, addr_len);
		if (client_sockfd == -1)
		{
			perror("accept");
			return -1;
		}
		else
		{
			return client_sockfd;  // accept successful
		}
	}
}

int receive_from(int sockfd, char* buf, int time)
{
    fd_set set;
	struct timeval timeout;
	FD_ZERO(&set);
	FD_SET(sockfd, &set);
	timeout.tv_sec = time;
	timeout.tv_usec = 0;
	
	int numbytes;
	int rv;

	rv = select(sockfd + 1, &set, NULL, NULL, &timeout);
	if (rv == -1)
	{
		perror("select");
		return -1;
	}
	else if (rv == 0)
		return -2;  // timeout
	else
	{
		numbytes = recv(sockfd, buf, MAXBUFLEN - 1, 0);
		if (numbytes == -1)
		{
			perror("read");
			return -1;
		}
		else if (numbytes == 0)
			return 0;  // disconnect
		else
		{
			return numbytes;  // read successful
		}
	}

    numbytes = recv(sockfd, buf, MAXBUFLEN - 1, 0);
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
    (*shm_iter) += (port - LISTENPORT - 1);
}
