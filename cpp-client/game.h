#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "player.h"
#include "client.h"
#include <signal.h>

class Game
{
public:
	Game();
	~Game() {}
	void start();
private:
	Board m_board;
	Player m_p1;
	Player m_p2;
    static void* check_giveup(void* parameters);
    static void* check_sigint(void* parameters);
    static void sigint_handler(int s);
    static sig_atomic_t sigint_check;
    char m_recv_buf[MAXBUFLEN];
};

#endif
