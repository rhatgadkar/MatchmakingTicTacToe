#ifndef CONNECTION_H
#define CONNECTION_H

#include <netinet/in.h>

#define LISTENPORT 4950  // the port clients will be connecting to
#define MAXBUFLEN 100
#define BACKLOG 20
#define MAX_CHILD_SERVERS 100
#define SHM_POP_POS 900
#define SHM_LOCK_POS 899

int setup_connection(int* sockfd, struct addrinfo* servinfo, int port_int);

// Return 0 if able to send port to client. Otherwise return -1.
int handle_syn_port(int sockfd, int* curr_port, int* client_port,
		int* shm_ports_used, int* sockfd_client);

void handle_match_msg(int sockfd, int* shm_iter, int* shm_ports_used);

void port_to_shm_iter(int port, int** shm_iter, int* shm_ports_used);

void acquire_shm_lock(int* shm_ports_used);

void release_shm_lock(int* shm_ports_used);

#endif  // CONNECTION_H
