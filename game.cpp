#include "game.h"
#include <iostream>
using namespace std;

Game::Game()
	: m_p1('x'), m_p2('o')
{
}

void Game::start()
{
	bool p1turn = true;
	for (;;)
	{
		// draw board
		if (p1turn)
			cout << "Player 1 turn" << endl;
		else
			cout << "Player 2 turn" << endl;
		m_board.draw();
		
		// insert at position
		int input;
		for (;;)
		{
			cout << "Enter position (1-9): ";
			cin >> input;

			if (p1turn && !m_board.insert(m_p1.getSymbol(), input))
				continue;
			if (!p1turn && !m_board.insert(m_p2.getSymbol(), input))
				continue;
			break;
		}

		// check if win
		if (m_board.isWin(input))
		{
			if (p1turn)
				cout << "Player 1 wins" << endl;
			else
				cout << "Player 2 wins" << endl;
			m_board.draw();
			return;
		}

		p1turn = !p1turn;
	}
}
