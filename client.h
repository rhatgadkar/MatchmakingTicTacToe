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
private:
    // variables
    struct addrinfo *m_p;
    struct addrinfo *m_servinfo;
    int m_sockfd;
    // functions

};

#endif  // CLIENT_H
