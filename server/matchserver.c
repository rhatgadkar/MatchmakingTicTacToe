#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include "connection.h"
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>

#define FIFO_NAME "fifo"
#define SHM_SIZE 4096

int fifo_fd;
int* shm_ports_used;

void sigchld_handler(int s)
{
    int status;
    char buf[MAXBUFLEN];
    int port;
    int* shm_iter;

    
    while(waitpid(-1, NULL, WNOHANG) > 0)
    {
        memset(buf, 0, MAXBUFLEN);
        status = read(fifo_fd, buf, 4);
        if (status == -1)
            perror("sigchld read");

        port = (int)strtol(buf, (char**)NULL, 10);
        
        printf("clearing port: %d\n", port);
        port_to_shm_iter(port, &shm_iter, shm_ports_used);
        *shm_iter = 0;
    }
}

void initialize_shm()
{
    int shmid;
    key_t key;
    int* shm_iter;

    key = 5678;

    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }

    shm_ports_used = (int*)shmat(shmid, 0, 0);
    if (shm_ports_used == (int*)-1)
    {
        perror("shmat");
        exit(1);
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

        initialize_shm();
        int* shm_iter;
        port_to_shm_iter(curr_port, &shm_iter, shm_ports_used);

        status = setup_connection(&sockfd, servinfo, curr_port);
        if (status != 0)
        {
            printf("Child server at port: %d is already running.\n",
                   curr_port);
            exit(1);
        }
        
        int fd;
        struct flock lock_w;
        char str_curr_port[MAXBUFLEN];
        sprintf(str_curr_port, "%d", curr_port);

        handle_match_msg(sockfd, shm_iter);

        printf("Child server at port: %d has closed.\n", curr_port);
		
        mkfifo(FIFO_NAME, S_IFIFO | 0666);
        fifo_fd = open(FIFO_NAME, O_WRONLY);
		if (fifo_fd == -1)
		{
			perror("open fifo in child");
			exit(1);
		}
        
        printf("writing port: %s\n", str_curr_port);
        status = write(fifo_fd, str_curr_port, 4);
        if (status == -1)
            perror("create_match_server write");

		close(fifo_fd);

        close(sockfd);
//        freeaddrinfo(servinfo);
        exit(0);
    }
}

int main()
{
    int status;
    int sockfd;
    int sockfd_client;
    struct addrinfo* servinfo;

    initialize_shm();
    int* shm_iter;
    shm_iter = shm_ports_used;
    int k;
    for (k = 0; k < 1000; k++)
        *shm_iter++ = 0;

    status = setup_connection(&sockfd, servinfo, LISTENPORT);
    if (status != 0)
    {
        printf("Parent server cannot start.\n");
        return status;
    }

    int curr_port;
    int client_port;

    client_port = LISTENPORT + 1;

    if ((mkfifo(FIFO_NAME, S_IFIFO | 0666)) == -1)
	{
		perror("parent mkfifo");
		exit(1);
	}
    fifo_fd = open(FIFO_NAME, O_RDONLY | O_NDELAY);
	if (fifo_fd == -1)
	{
		perror("open fifo in parent");
		exit(1);
	}

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
        status = handle_syn_port(sockfd, &curr_port, &client_port,
                                 shm_ports_used, &sockfd_client);
        close(sockfd_client);
        if (status == -1)
            continue;

        port_to_shm_iter(curr_port, &shm_iter, shm_ports_used);
        if (*shm_iter == 0)
            create_match_server(curr_port);
    }
    
    close(sockfd);
    freeaddrinfo(servinfo);
	return 0;
}
