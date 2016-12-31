#ifndef CLIENT_H
#define CLIENT_H

#define SERVERIP "54.183.217.40"
//#define SERVERIP "192.168.218.140"
#define SERVERPORT "4950"
#define MAXBUFLEN 100

#include <netdb.h>
#include <string>

class Client
{
public:
	Client(std::string username, std::string password);
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
	std::string m_username;
	std::string m_password;
	// functions
	int create_socket_server(const char* port);
	bool handle_syn_ack(char resp[MAXBUFLEN]);  // return port of child server
	bool handle_child_syn_ack(char resp[MAXBUFLEN]);
	bool get_num_ppl();
	int send_to_server(const char* text);
	int receive_from(char* buf, int time);
};

#endif  // CLIENT_H
