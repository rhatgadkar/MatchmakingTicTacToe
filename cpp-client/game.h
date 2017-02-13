#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "player.h"
#include "client.h"
#include <string>
#include <pthread.h>

class Game
{
public:
	Game();
	~Game() {}
	void start(std::string username, std::string password);
private:
	Board m_board;
	Player m_p1;
	Player m_p2;
	struct check_giveup_params
	{
		Client* c;
		char* recv_buf;
		Board* board;
	};
	struct timer_params
	{
		int seconds;
		int* got_move;
		std::string msg;
		Client* c;
		bool giveup;
		pthread_t* giveup_t;
	};
	static void* check_giveup(void* parameters);
	static void* timer_countdown(void* parameters);
	char m_recv_buf[MAXBUFLEN];
	static pthread_mutex_t recv_buf_mutex;
};

#endif
