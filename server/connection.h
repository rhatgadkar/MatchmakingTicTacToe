#ifndef CONNECTION_H
#define CONNECTION_H

#include <netinet/in.h>

#define LISTENPORT 4950  // the port clients will be connecting to
#define MAXBUFLEN 100
#define BACKLOG 20

int setup_connection(int* sockfd, struct addrinfo* servinfo, int port_int);

void handle_syn_port(int sockfd, int* curr_port, int* client_port,
                     int* shm_ports_used, int* sockfd_client);

void handle_match_msg(int sockfd, int* shm_iter);

void port_to_shm_iter(int port, int** shm_iter, int* shm_ports_used);

#endif  // CONNECTION_H
