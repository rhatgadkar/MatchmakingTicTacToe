#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "player.h"

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
};

#endif
