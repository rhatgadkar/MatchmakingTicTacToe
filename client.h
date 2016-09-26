#ifndef CLIENT_H
#define CLIENT_H

#define SERVERIP "127.0.0.1"
#define SERVERPORT "4950"
#define MAXBUFLEN 100

#include <netdb.h>

class Client
{
public:
    Client();
    ~Client();
    int send_position(int pos);
    int receive_position();
    bool is_p1() { return m_is_p1; }
private:
    // variables
    bool m_is_p1;
    struct addrinfo* m_p;
    struct addrinfo* m_servinfo;
    int m_sockfd;
    char m_rcv_buf[MAXBUFLEN];
    // functions
    static void* timer_countdown(void* parameters);
    int create_socket_server(const char* port);
    void handle_syn_ack(char resp[MAXBUFLEN]);  // return port of child server
};

#endif  // CLIENT_H
