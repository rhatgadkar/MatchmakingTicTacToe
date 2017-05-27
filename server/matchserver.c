#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include "connection.h"
#include <sys/ipc.h>
#include <sys/stat.h>
#include "queue.h"

#define FIFO_NAME "fifo"

int fifo_fd;

void* free_child_processes(void* parameters)
{
	int status;
	char buf[MAXBUFLEN];
	int port;
	int* port_iter;

	struct server_pop* sp = (struct server_pop*)parameters;

	for(;;)
	{
		sleep(10);

		while ((waitpid(-1, NULL, WNOHANG)) > 0)
		{
			memset(buf, 0, MAXBUFLEN);
			status = read(fifo_fd, buf, 4);
			if (status == -1)
				perror("free_child_processes read");

			port = (int)strtol(buf, (char**)NULL, 10);

			if (port != 0)
			{
				printf("clearing port: %d\n", port);
				port_to_array_iter(port, &port_iter,
						sp->child_server_pop);
				pthread_mutex_lock(&(sp->mutex));
				sp->total_pop -= *port_iter;
				*port_iter = 0;
				push_queue(sp->empty_servers, port);
				pthread_mutex_unlock(&(sp->mutex));
			}
		}
	}

	return NULL;
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

		status = setup_connection(&sockfd, servinfo, curr_port);
		if (status != 0)
		{
			printf("Child server at port: %d is already running.\n",
					curr_port);
			exit(1);
		}

		int fd;
		char str_curr_port[MAXBUFLEN];
		sprintf(str_curr_port, "%d", curr_port);

		handle_match_msg(sockfd);

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
		// freeaddrinfo(servinfo);
		exit(0);
	}
}

int main()
{
	int status;
	int sockfd;
	int sockfd_client;
	struct addrinfo* servinfo;
	int child_fifo_fd;
	int k;

	struct server_pop sp;
	pthread_mutex_init(&(sp.mutex), NULL);
	for (k = 0; k < MAX_CHILD_SERVERS; k++)
		sp.child_server_pop[k] = 0;
	sp.total_pop = 0;
	struct queue* empty_servers = create_empty_queue();
	for (k = LISTENPORT + 1; k < LISTENPORT + 1 + MAX_CHILD_SERVERS; k++)
		push_queue(empty_servers, k);
	sp.empty_servers = empty_servers;

	status = setup_connection(&sockfd, servinfo, LISTENPORT);
	if (status != 0)
	{
		printf("Parent server cannot start.\n");
		return status;
	}

	int curr_port;

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

	struct queue* waiting_servers = create_empty_queue();

	pthread_t free_child_processes_thread;
	pthread_create(&free_child_processes_thread, NULL,
			&free_child_processes, &sp);

	int* port_iter;

	for (;;)
	{
		status = handle_syn_port(sockfd, &curr_port, &sockfd_client,
				waiting_servers, &sp);
		close(sockfd_client);
		if (status == -1)
			continue;

		port_to_array_iter(curr_port, &port_iter, sp.child_server_pop);
		pthread_mutex_lock(&(sp.mutex));
		if (*port_iter == 1)
		{
			pthread_mutex_unlock(&(sp.mutex));
			create_match_server(curr_port);
		}
		else
		{
			pthread_mutex_unlock(&(sp.mutex));
		}
	}

	close(sockfd);
	freeaddrinfo(servinfo);
	return 0;
}
