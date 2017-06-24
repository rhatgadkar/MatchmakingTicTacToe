#ifndef CONNECTION_H
#define CONNECTION_H

#include "queue.h"
#include <netinet/in.h>

#define LISTENPORT 4950  // the port clients will be connecting to
#define MAXBUFLEN 100
#define BACKLOG 20
#define MAX_CHILD_SERVERS 250

struct server_pop
{
	int child_server_pop[MAX_CHILD_SERVERS];
	int total_pop;
	struct queue* empty_servers;
	pthread_mutex_t mutex;
};

int setup_connection(int* sockfd, struct addrinfo* servinfo, int port_int);

// Return 0 if able to send port to client. Otherwise return -1.
int handle_syn_port(int sockfd, int* curr_port, int* sockfd_client,
		struct queue* waiting_servers, struct server_pop* sp);

void handle_match_msg(int sockfd);

void port_to_array_iter(int port, int** array_iter, int* array_ports);

#endif  // CONNECTION_H
