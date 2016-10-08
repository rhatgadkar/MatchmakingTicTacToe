#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <map>
#include <pthread.h>
#include <fcntl.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include "connection.h"
using namespace std;

const char* file = "file.txt";

struct kill_zombie_params
{
    map<int, int>* ports_used;
};

void* kill_zombie_process(void* parameters)
{
    if (parameters == NULL)
    {
        cerr << "params is NULL" << endl;
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
            pthread_mutex_lock(&ports_used_mutex);
            params->ports_used->erase(port);
            pthread_mutex_unlock(&ports_used_mutex);
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
    map<int, int> ports_used;

    client_port = LISTENPORT + 1;

    struct kill_zombie_params zombie_params;
    zombie_params.ports_used = &ports_used;
    pthread_t kill_zombie_id;
    pthread_create(&kill_zombie_id, NULL, &kill_zombie_process,
                   &zombie_params);

    for (;;)
    {
        handle_syn_port(sockfd, curr_port, client_port, ports_used);

        pthread_mutex_lock(&ports_used_mutex);
        if (ports_used[curr_port] == 1)
            create_match_server(curr_port);
        pthread_mutex_unlock(&ports_used_mutex);
    }
    
    close(sockfd);
    freeaddrinfo(servinfo);
	return 0;
}
