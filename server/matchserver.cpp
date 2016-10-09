#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <map>
#include <signal.h>
#include <fcntl.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include "connection.h"
using namespace std;

const char* file = "file.txt";

map<int, int> ports_used;

void sigchld_handler(int s)
{
    int fd;
    struct flock lock_r;
    int status;
    char buf[MAXBUFLEN];
    int port;
    
    while(waitpid(-1, NULL, WNOHANG) > 0)
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
            close(fd);
            return;
        }
        close(fd);
        port = (int)strtol(buf, (char**)NULL, 10);
        ports_used.erase(port);
    }
}

void create_match_server(int curr_port)
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
        struct addrinfo* servinfo;
 
        status = setup_connection(sockfd, servinfo, curr_port);
        if (status != 0)
        {
            cout << "hmm" << endl;
            exit(1);
        }

        
        int fd;
        struct flock lock_w;
        char str_curr_port[MAXBUFLEN];
        sprintf(str_curr_port, "%d", curr_port);

        handle_match_msg(sockfd);
            
        fd = open(file, O_WRONLY | O_TRUNC);
        memset(&lock_w, 0, sizeof(lock_w));
        lock_w.l_type = F_WRLCK;
        fcntl(fd, F_SETLKW, &lock_w);
        status = write(fd, str_curr_port, strlen(str_curr_port));
        if (status == -1)
            perror("write port file");
        close(fd);

        close(sockfd);
        freeaddrinfo(servinfo);
        exit(0);
    }
}

int main()
{
    int status;
    int sockfd;
    struct addrinfo* servinfo;

    status = setup_connection(sockfd, servinfo, LISTENPORT);
    if (status != 0)
    {
        cout << "something went wrong" << endl;
        return status;
    }

    int curr_port;
    int client_port;

    client_port = LISTENPORT + 1;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigchld_handler;
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    for (;;)
    {
        handle_syn_port(sockfd, curr_port, client_port, ports_used);

        if (ports_used[curr_port] == 1)
            create_match_server(curr_port);
    }
    
    close(sockfd);
    freeaddrinfo(servinfo);
	return 0;
}
