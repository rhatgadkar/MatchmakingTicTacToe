#include "game.h"
#include "client.h"
#include <iostream>
using namespace std;

Game::Game()
	: m_p1('x'), m_p2('o')
{
}

void Game::start()
{
    Client c;

	bool p1turn = true;
	for (;;)
	{
		// draw board
		if (p1turn && c.is_p1())
			cout << "Your turn." << endl;
		else if (p1turn && !c.is_p1())
			cout << "Player 1 turn." << endl;
        else if (!p1turn && c.is_p1())
            cout << "Player 2 turn." << endl;
        else
            cout << "Your turn." << endl;
		m_board.draw();
		
		int input;
        // insert at position
        if ((p1turn && c.is_p1()) || (!p1turn && !c.is_p1()))
        {
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
            int res = c.send_position(input);
            if (res == -1)
            {
                cout << "error with send_position" << endl;
                return;
            }
        }
        // wait for other player to make move
        else
        {
            input = c.receive_position();
            if (p1turn && !m_board.insert(m_p1.getSymbol(), input))
            {
                cout << "error with receive_position" << endl;
                return;
            }
            if (!p1turn && !m_board.insert(m_p2.getSymbol(), input))
            {
                cout << "error with receive_position" << endl;
                return;
            }
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
