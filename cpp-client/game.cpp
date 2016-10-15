#include "game.h"
#include "client.h"
#include <iostream>
#include <signal.h>
#include <pthread.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
using namespace std;

Game::Game()
	: m_p1('x'), m_p2('o')
{
    memset(m_recv_buf, 0, MAXBUFLEN);
}

struct check_giveup_params
{
    Client* c;
    char* recv_buf;
};

void* Game::check_giveup(void* parameters)
{
    struct check_giveup_params* params;
    params = (struct check_giveup_params*)parameters;

    do
    {
        memset(params->recv_buf, 0, MAXBUFLEN);
        params->c->receive_from_server(params->recv_buf);
    } while(strcmp(params->recv_buf, "giveup") != 0);

    if (params->c->is_p1())
        cout << "Player 2 has given up.  Player 1 wins." << endl;
    else
        cout << "Player 1 has given up.  Player 2 wins." << endl;
    exit(0);
}

void Game::start()
{
    Client c;

    pthread_t giveup_t;
    struct check_giveup_params params;
    params.c = &c;
    params.recv_buf = m_recv_buf;
    pthread_create(&giveup_t, NULL, &(Game::check_giveup), &params);

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
            if (!c.send_position(input))
            {
                cout << "error with send_position" << endl;
                return;
            }
        }
        // wait for other player to make move
        else
        {
            for (;;)
            {
                if (m_recv_buf[0] == 0)
                    continue;
                if (isdigit(m_recv_buf[0]) && m_recv_buf[0] != '0')
                {
                    input = m_recv_buf[0] - '0';
                    break;
                }
            }
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
