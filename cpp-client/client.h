#ifndef CLIENT_H
#define CLIENT_H

#define SERVERIP "54.183.217.40"
//#define SERVERIP "127.0.0.1"
#define SERVERPORT "4950"
#define MAXBUFLEN 100

#include <netdb.h>

class Client
{
public:
	Client();
	~Client();
	bool send_position(int pos);
	bool is_p1() { return m_is_p1; }
	bool send_giveup();
	bool send_bye();
	bool send_win(int pos);
	int receive_from_server(char* buf);
private:
	// variables
	bool m_is_p1;
	struct addrinfo* m_p;
	struct addrinfo* m_servinfo;
	int m_sockfd;
	// functions
	int create_socket_server(const char* port);
	bool handle_syn_ack(char resp[MAXBUFLEN]);  // return port of child server
	bool get_num_ppl();
	int send_to_server(const char* text);
	int receive_from(char* buf, int time);
};

#endif  // CLIENT_H
