#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <map>
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
    hints.ai_socktype = SOCK_DGRAM;
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
    return 0;
}

void handle_syn_port(int sockfd, int& curr_port, int& client_port,
                     map<int, int>& ports_used)
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

    status = receive_from(sockfd, buf, MAXBUFLEN-1, &their_addr);
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
    status = send_to_address(sockfd, port, &their_addr);
    if (status == -1)
        perror("sendto ACK");

    cout << "Sent ACK to use port: " << port << endl;
}

void handle_match_msg(int sockfd)
{
    int status;

    struct sockaddr_in *their_addr_v4 = NULL;
    
    struct sockaddr_in first_addr;
    struct sockaddr_in second_addr;
    memset(&first_addr, 0, sizeof(first_addr));
    memset(&second_addr, 0, sizeof(second_addr));
    bool found_first_addr = false;
    bool found_second_addr = false;

    struct sockaddr_in *first_addr_v4 = NULL;
    struct sockaddr_in *second_addr_v4 = NULL;

    char their_addr_str[INET_ADDRSTRLEN];
    char first_addr_str[INET_ADDRSTRLEN];
    char second_addr_str[INET_ADDRSTRLEN];

    struct sockaddr their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len = sizeof(their_addr);
    char s[INET_ADDRSTRLEN];

    for (;;)
    {
        memset(buf, 0, MAXBUFLEN);

        cout << "Waiting to recvfrom..." << endl;

        status = receive_from(sockfd, buf, MAXBUFLEN-1, &their_addr);
        if (status == -1)
        {
            perror("recvfrom");
        }
        their_addr_v4 = (struct sockaddr_in *)&their_addr;

        inet_ntop(AF_INET, &(their_addr_v4->sin_addr), their_addr_str,
                  sizeof(their_addr_str));

        cout << "Got packet from " << their_addr_str << ", "
             << their_addr_v4->sin_port << endl;

        if (strcmp(buf, "bye") == 0)
        {
            if (found_first_addr && !found_second_addr)
            {
                cout << "Received 'bye', closing connection to "
                     << first_addr_str << ", " << first_addr_v4->sin_port
                     << endl;
            }
            else if (found_first_addr && found_second_addr)
            {
                cout << "Received 'bye', closing connection to "
                     << first_addr_str << ", " << first_addr_v4->sin_port
                     << endl;
                cout << "Received 'bye', closing connection to "
                     << second_addr_str << ", "
                     << second_addr_v4->sin_port << endl;

                if (memcmp(&first_addr, &their_addr, sizeof(their_addr)) == 0)
                {
                    // send bye to second address
                    status = send_to_address(sockfd, "bye",
                                           (struct sockaddr*)&second_addr);
                    if (status == -1)
                    {
                        perror("server: sendto");
                        exit(1);
                    }
                }
                else if (memcmp(&second_addr, &their_addr, sizeof(their_addr)) == 0)
                {
                    // send bye to first address
                    status = send_to_address(sockfd, "bye",
                                           (struct sockaddr*)&first_addr);
                    if (status == -1)
                    {
                        perror("server: sendto");
                        exit(1);
                    }
                }
            }
            else
            {
                cout << "Received 'bye', closing connection to "
                     << their_addr_str << ", " << their_addr_v4->sin_port
                     << endl;

            }
            return;
        }
        else
        {
            if (!found_first_addr)
            {
                found_first_addr = true;
                memcpy(&first_addr, &their_addr, sizeof(their_addr));
                first_addr_v4 = (struct sockaddr_in*)&first_addr;

                inet_ntop(AF_INET, &(first_addr_v4->sin_addr),
                          first_addr_str, sizeof(first_addr_str));
        
                // send ACK to client 1 (player 1)
                status = send_to_address(sockfd, "player-1",
                                       (struct sockaddr*)&first_addr);
                if (status == -1)
                {
                    perror("server: ACK to first_addr");
                    exit(1);
                }

                cout << "Waiting for second client to connect..." << endl;
            }
            else if (found_first_addr && !found_second_addr &&
                     memcmp(&first_addr, &their_addr, sizeof(their_addr)) != 0)
            {
                found_second_addr = true;
                memcpy(&second_addr, &their_addr, sizeof(their_addr));
                second_addr_v4 = (struct sockaddr_in*)&second_addr;

                inet_ntop(AF_INET, &(second_addr_v4->sin_addr),
                          second_addr_str, sizeof(second_addr_str));

                // send ACK to client 2 (player 2)
                status = send_to_address(sockfd, "player-2",
                                       (struct sockaddr*)&second_addr);
                if (status == -1)
                {
                    perror("server: ACK to second_addr");
                    exit(1);
                }
                status = send_to_address(sockfd, "player-2",
                                       (struct sockaddr*)&first_addr);
                if (status == -1)
                {
                    perror("server: ACK to first_addr");
                    exit(1);
                }

                printf("Second client connected.\n");
            }
            else if (found_first_addr && found_second_addr)
            {
                if (memcmp(&first_addr, &their_addr, sizeof(their_addr)) == 0)
                {
                    // forward message from first_addr to second_addr
                    status = send_to_address(sockfd, buf,
                                           (struct sockaddr*)&second_addr);
                    if (status == -1)
                    {
                        perror("server: forward to second_addr");
                        exit(1);
                    }
                }
                else if (memcmp(&second_addr, &their_addr, sizeof(their_addr)) == 0)
                {
                    // forward message from second_addr to first_addr
                    status = send_to_address(sockfd, buf,
                                           (struct sockaddr*)&first_addr);
                    if (status == -1)
                    {
                        perror("server: forward to first_addr");
                        exit(1);
                    }
                }
                else
                {
                    cerr << "It shouldn't go here" << endl;
                }
            }
            else if (found_first_addr && !found_second_addr)
            {
                cout << "Waiting for second client to connect..." << endl;
            }
        }
    }
}

int receive_from(int sockfd, char* buf, size_t size,
                 struct sockaddr* their_addr)
{
    int numbytes;
    socklen_t addr_len;

    addr_len = sizeof(*their_addr);
    numbytes = recvfrom(sockfd, buf, size, 0, their_addr, &addr_len);

    if (numbytes == -1)
        return -1;
    return 0;
}

int send_to_address(int sockfd, const char* text, struct sockaddr* their_addr)
{
    int numbytes;
    socklen_t addr_len;

    addr_len = sizeof(*their_addr);
    numbytes = sendto(sockfd, text, strlen(text), 0, their_addr, addr_len);
    
    if (numbytes == -1)
        return -1;
    return 0;
}
