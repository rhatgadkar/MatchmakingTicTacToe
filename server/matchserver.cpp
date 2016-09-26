//#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
//#include <string>
#include <string.h>
#include <sys/socket.h>
//#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <map>
#include <pthread.h>
#include <fcntl.h>

#define LISTENPORT_STR "4950"  // the port clients will be connecting to
#define LISTENPORT_INT 4950
#define MAXBUFLEN 100

int setup_connection(const char *port, int &sockfd)
{
    int status;
    struct addrinfo hints, *servinfo, *p;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;  // use my IP

    if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
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
        fprintf(stderr, "Failed to bind socket\n");
        return 2;
    }
    return 0;
}

const char* file = "file.txt";

struct kill_zombie_params
{
    std::map<int, int>* ports_used;
};

void* kill_zombie_process(void* parameters)
{
    if (parameters == NULL)
    {
        fprintf(stderr, "params is NULL\n");
        exit(1);
    }
    
    int fd;
    struct flock lock_r;
    int status;
    char buf[MAXBUFLEN];
    int port;
    
    struct kill_zombie_params* params = (struct kill_zombie_params*)parameters;

    for (;;)
    {
        waitpid(-1, &status, WNOHANG);  // kill zombie process
        if (WIFEXITED(status))
        {
            fd = open(file, O_RDONLY);
            memset(&lock_r, 0, sizeof(lock_r));
            lock_r.l_type = F_RDLCK;
            fcntl(fd, F_SETLKW, &lock_r);
            memset(buf, 0, MAXBUFLEN);
            status = read(fd, buf, MAXBUFLEN);
            if (status == -1)
            {
                perror("read port file");
                return NULL;
            }
            close(fd);
            port = (int)strtol(buf, (char**)NULL, 10);
            params->ports_used->erase(port);
        }
    }
    return NULL;
}

void create_child_process(int curr_port)
{
    pid_t child_pid;

    child_pid = fork();
    if (child_pid != 0)
    {
        // parent
    }
    else
    {
        // child
        int sockfd;
        int status;
 
        char str_curr_port[10];
        sprintf(str_curr_port, "%d", curr_port);
        
        status = setup_connection(str_curr_port, sockfd);
        if (status != 0)
        {
            printf("hmm\n");
            exit(1);
        }

        int numbytes;
        struct sockaddr their_addr;
        char buf[MAXBUFLEN];
        socklen_t addr_len = sizeof(their_addr);
        char s[INET_ADDRSTRLEN];
        
        int fd;
        struct flock lock_w;

        struct sockaddr_in *their_addr_v4 = NULL;
        
        struct sockaddr_in first_addr;
        struct sockaddr_in second_addr;
        memset(&first_addr, 0, sizeof(first_addr));
        memset(&second_addr, 0, sizeof(second_addr));
        bool found_first_addr = false;
        bool found_second_addr = false;

        struct sockaddr_in *first_addr_v4 = NULL;
        struct sockaddr_in *second_addr_v4 = NULL;

        for (;;)
        {
            printf("Waiting to recvfrom...\n");

            if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                                     &their_addr, &addr_len)) == -1)
            {
                perror("recvfrom");
            }
            buf[numbytes] = '\0';
            their_addr_v4 = (struct sockaddr_in *)&their_addr;
            printf("Got packet from %s, %hu\n", inet_ntop(AF_INET,
                                                  &(their_addr_v4->sin_addr),
                                                  s, sizeof(s)),
                                                  their_addr_v4->sin_port);
            if (strcmp(buf, "bye") == 0)
            {
                if (found_first_addr && !found_second_addr)
                {
                    printf("Received 'bye', closing connection to %s, %hu.\n",
                                            inet_ntop(AF_INET,
                                                      &(first_addr_v4->sin_addr),
                                                      s, sizeof(s)),
                                                      first_addr_v4->sin_port);
                }
                else if (found_first_addr && found_second_addr)
                {
                    printf("Received 'bye', closing connection to %s, %hu.\n",
                                            inet_ntop(AF_INET,
                                                      &(first_addr_v4->sin_addr),
                                                      s, sizeof(s)),
                                                      first_addr_v4->sin_port);
                    printf("Received 'bye', closing connection to %s, %hu.\n",
                                            inet_ntop(AF_INET,
                                                      &(second_addr_v4->sin_addr),
                                                      s, sizeof(s)),
                                                      second_addr_v4->sin_port);
                    if (memcmp(&first_addr, &their_addr, sizeof(their_addr)) == 0)
                    {
                        // send bye to second address
                        if ((numbytes = sendto(sockfd, buf, numbytes, 0,
                                 (struct sockaddr*)&second_addr, addr_len)) == -1)
                        {
                            perror("server: sendto");
                            exit(1);
                        }
                    }
                    else if (memcmp(&second_addr, &their_addr, sizeof(their_addr)) == 0)
                    {
                        // send bye to first address
                        if ((numbytes = sendto(sockfd, buf, numbytes, 0,
                                 (struct sockaddr*)&first_addr, addr_len)) == -1)
                        {
                            perror("server: sendto");
                            exit(1);
                        }
                    }
                }
                else
                {
                    printf("Received 'bye', closing connection to %s, %hu.\n",
                                            inet_ntop(AF_INET,
                                                      &(their_addr_v4->sin_addr),
                                                      s, sizeof(s)),
                                                      their_addr_v4->sin_port);

                }
                
                fd = open(file, O_WRONLY | O_TRUNC);
                memset(&lock_w, 0, sizeof(lock_w));
                lock_w.l_type = F_WRLCK;
                fcntl(fd, F_SETLKW, &lock_w);
                status = write(fd, str_curr_port, strlen(str_curr_port));
                if (status == -1)
                    perror("write port file");
                close(fd);

                close(sockfd);
//                freeaddrinfo(servinfo);
                exit(0);
            }
            else
            {
                if (!found_first_addr)
                {
                    found_first_addr = true;
                    memcpy(&first_addr, &their_addr, sizeof(their_addr));
                    first_addr_v4 = (struct sockaddr_in*)&first_addr;

                    // send ACK to client 1 (player 1)
                    if ((numbytes = sendto(sockfd, "player-1",
                                           strlen("player-1"), 0,
                                           (struct sockaddr*)&first_addr,
                                           addr_len)) == -1)
                    {
                        perror("server: ACK to first_addr");
                        exit(1);
                    }

                    printf("Waiting for second client to connect...\n");
                }
                else if (found_first_addr && !found_second_addr &&
                         memcmp(&first_addr, &their_addr, sizeof(their_addr)) != 0)
                {
                    found_second_addr = true;
                    memcpy(&second_addr, &their_addr, sizeof(their_addr));
                    second_addr_v4 = (struct sockaddr_in*)&second_addr;

                    // send ACK to client 2 (player 2)
                    if ((numbytes = sendto(sockfd, "player-2",
                                           strlen("player-2"), 0,
                                           (struct sockaddr*)&second_addr,
                                           addr_len)) == -1)
                    {
                        perror("server: ACK to second_addr");
                        exit(1);
                    }
                    if ((numbytes = sendto(sockfd, "player-2",
                                           strlen("player-2"), 0,
                                           (struct sockaddr*)&first_addr,
                                           addr_len)) == -1)
                    {
                        perror("server: ACK to first_addr for second_addr");
                        exit(1);
                    }

                    printf("Second client connected.\n");
                }
                else if (found_first_addr && found_second_addr)
                {
                    if (memcmp(&first_addr, &their_addr, sizeof(their_addr)) == 0)
                    {
                        // forward message from first_addr to second_addr
                        if ((numbytes = sendto(sockfd, buf, numbytes, 0,
                                 (struct sockaddr*)&second_addr, addr_len)) == -1)
                        {
                            perror("server: forward to second_addr");
                            exit(1);
                        }
                    }
                    else if (memcmp(&second_addr, &their_addr, sizeof(their_addr)) == 0)
                    {
                        // forward message from second_addr to first_addr
                        if ((numbytes = sendto(sockfd, buf, numbytes, 0,
                                 (struct sockaddr*)&first_addr, addr_len)) == -1)
                        {
                            perror("server: forward to first_addr");
                            exit(1);
                        }
                    }
                    else
                    {
                        fprintf(stderr, "It shouldn't go here\n");
                    }
                }
                else if (found_first_addr && !found_second_addr)
                {
                    printf("Waiting for second client to connect...\n");
/*                    char* msg = "Waiting for Player 2 to connect...";
                    if ((numbytes = sendto(sockfd, msg, strlen(msg), 0,
                             (struct sockaddr*)&first_addr, addr_len)) == -1)
                    {
                        perror("server: forward waiting to first_addr");
                        exit(1);
                    }*/
                }
            }
        }
    }
}

int main()
{
    int sockfd;
    int status;
    
    status = setup_connection(LISTENPORT_STR, sockfd);
    if (status != 0)
    {
        printf("something went wrong\n");
        return status;
    }

	int numbytes;
	struct sockaddr their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len = sizeof(their_addr);
    char s[INET_ADDRSTRLEN];

    int client_port = LISTENPORT_INT + 1;
    std::map<int, int> ports_used;

    struct kill_zombie_params zombie_params;
    zombie_params.ports_used = &ports_used;
    pthread_t kill_zombie_id;
    pthread_create(&kill_zombie_id, NULL, &kill_zombie_process,
                   &zombie_params);

    for (;;)
    {
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                                 &their_addr, &addr_len)) == -1)
        {
            perror("recvfrom SYN");
        }
        buf[numbytes] = '\0';
        if (strcmp(buf, "SYN") == 0)
        {
            struct sockaddr_in *their_addr_v4;
            their_addr_v4 = (struct sockaddr_in *)&their_addr;
            printf("Got SYN from %s, %hu\n", inet_ntop(AF_INET,
                                                  &(their_addr_v4->sin_addr), s,
                                                  sizeof(s)),
                                                  their_addr_v4->sin_port);
            int curr_port = LISTENPORT_INT + 1;
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
                curr_port = LISTENPORT_INT + 1;
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
            
            char port[10];
            sprintf(port, "%d", curr_port);
            if (sendto(sockfd, port, strlen(port), 0,
                       &their_addr, addr_len) == -1)
            {
                perror("sendto ACK");
            }
            printf("Sent ACK to use port: %s\n", port);
            
            if (ports_used[curr_port] == 1)
                create_child_process(curr_port);
        }
    }
    
    close(sockfd);
//    freeaddrinfo(servinfo);
	return 0;
}
