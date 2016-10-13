#ifndef CONNECTION_H
#define CONNECTION_H

#include <map>
#include <netinet/in.h>

#define LISTENPORT 4950  // the port clients will be connecting to
#define MAXBUFLEN 100
#define BACKLOG 20

static int receive_from(int sockfd, char* buf, size_t size);

static int send_to_address(int sockfd, const char* text);

int setup_connection(int& sockfd, struct addrinfo* servinfo, int port_int);

void handle_syn_port(int sockfd, int& curr_port, int& client_port,
                     std::map<int, int>& ports_used, int& sockfd_client);

void handle_match_msg(int sockfd);

#endif  // CONNECTION_H
