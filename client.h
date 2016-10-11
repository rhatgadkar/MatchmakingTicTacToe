#ifndef CLIENT_H
#define CLIENT_H

#define SERVERIP "54.183.217.40"
#define SERVERPORT "4950"
#define MAXBUFLEN 100

#include <netdb.h>
#include <signal.h>

class Client
{
public:
    Client();
    ~Client();
    bool send_position(int pos);
    int receive_position();
    bool is_p1() { return m_is_p1; }
    bool send_giveup();
    bool receive_giveup();
private:
    // variables
    bool m_is_p1;
    struct addrinfo* m_p;
    struct addrinfo* m_servinfo;
    int m_sockfd;
    static sig_atomic_t sigint_check;
    // functions
    bool send_bye();
    static void* timer_countdown(void* parameters);
    int create_socket_server(const char* port);
    void handle_syn_ack(char resp[MAXBUFLEN]);  // return port of child server
    int send_to_server(const char* text);
    int receive_from_server(char* buf, size_t size);
    static void* check_sigint(void* parameters);
    static void sigint_handler(int s);
    static void sigint_ignore_handler(int s);
};

#endif  // CLIENT_H
