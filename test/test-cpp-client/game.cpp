#include "game.h"
#include "client.h"
#include <iostream>
#include <signal.h>
#include <pthread.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <time.h>
#include <string>
#include <unistd.h>
using namespace std;

pthread_mutex_t Game::recv_buf_mutex = PTHREAD_MUTEX_INITIALIZER;

Game::Game()
	: m_p1(Player::P1_SYMBOL), m_p2(Player::P2_SYMBOL)
{
	pthread_mutex_lock(&Game::recv_buf_mutex);
	memset(m_recv_buf, 0, MAXBUFLEN);
	pthread_mutex_unlock(&Game::recv_buf_mutex);
}

void* Game::check_giveup(void* parameters)
{
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	char test[MAXBUFLEN];
	struct check_giveup_params* params;
	params = (struct check_giveup_params*)parameters;

	pthread_mutex_lock(&Game::recv_buf_mutex);
	do
	{
		pthread_mutex_unlock(&Game::recv_buf_mutex);
		memset(test, 0, MAXBUFLEN);
		params->c->receive_from(test, 1);
		pthread_mutex_lock(&Game::recv_buf_mutex);
		strcpy(params->recv_buf, test);
		if (params->recv_buf != NULL && params->recv_buf[0] != 0 &&
				(params->recv_buf[0] == 'w' || params->recv_buf[0] == 't'))
		{
			if (params->recv_buf[0] == 'w' || params->recv_buf[0] == 't')
			{
				if (params->c->is_p1())
				{
					params->board->insert(Player::P2_SYMBOL, params->recv_buf[1] - '0');
					if (params->recv_buf[0] == 'w')
						cout << "Player 2 wins" << endl;
					else
						cout << "Tie game" << endl;
					pthread_mutex_unlock(&Game::recv_buf_mutex);
				}
				else
				{
					params->board->insert(Player::P1_SYMBOL, params->recv_buf[1] - '0');
					if (params->recv_buf[0] == 'w')
						cout << "Player 1 wins" << endl;
					else
						cout << "Tie game" << endl;
					pthread_mutex_unlock(&Game::recv_buf_mutex);
				}
			}
			else
				pthread_mutex_unlock(&Game::recv_buf_mutex);
			params->board->draw();
			cout << "Client is exiting.  Closing server." << endl;
			exit(0);
		}
		else
			pthread_mutex_unlock(&Game::recv_buf_mutex);
		pthread_mutex_lock(&Game::recv_buf_mutex);
	} while(strcmp(params->recv_buf, "giveup") != 0);
	pthread_mutex_unlock(&Game::recv_buf_mutex);

	if (params->c->is_p1())
		cout << "Player 2 has given up.  Player 1 wins." << endl;
	else
		cout << "Player 1 has given up.  Player 2 wins." << endl;
	exit(0);
}

void* Game::timer_countdown(void* parameters)
{
	time_t start;
	time_t end;
	struct timer_params* params = (struct timer_params*)parameters;

	time(&start);
	do
	{
		time(&end);
		if (*(params->got_move))
			return NULL;
	} while(difftime(end, start) < params->seconds);
	cout << params->msg << endl;
	if (params->giveup)
	{
		params->c->send_giveup();
		exit(0);
	}
	pthread_cancel(*params->giveup_t);
	pthread_join(*params->giveup_t, NULL);
	*(params->got_move) = 1;
	return NULL;
}

void Game::start(string username, string password, int* user_input,
					int user_input_length)
{
	int user_input_iter = 0;
	
	Client c(username, password);

	if (username != "" && password != "")
	{
		// print win/loss record
		int comma_pos = c.Record.find_first_of(",");
		string win_record = c.Record.substr(0, comma_pos);
		string loss_record = c.Record.substr(comma_pos + 1,
				c.Record.length() - (comma_pos + 1));
		cout << "W: " << win_record << endl;
		cout << "L: " << loss_record << endl;
	}

	pthread_t giveup_t;
	struct check_giveup_params params;
	params.c = &c;
	pthread_mutex_lock(&Game::recv_buf_mutex);
	params.recv_buf = m_recv_buf;
	pthread_mutex_unlock(&Game::recv_buf_mutex);
	params.board = &m_board;
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
			pthread_t timer_thread;
			int got_move = 0;

			struct timer_params params_timer;
			params_timer.seconds = 30;
			params_timer.got_move = &got_move;
			params_timer.msg =
			"You have not played a move in 30 seconds. You have given up.";
			params_timer.c = &c;
			params_timer.giveup = true;
			params_timer.giveup_t = &giveup_t;

			pthread_create(&timer_thread, NULL, &(Game::timer_countdown),
			&params_timer);
			for (;;)
			{
				sleep(1);
				if (user_input_iter >= user_input_length)
					continue;
				else
					input = user_input[user_input_iter];

				if (p1turn && !m_board.insert(m_p1.getSymbol(), input))
				{
					user_input_iter++;
					continue;
				}
				if (!p1turn && !m_board.insert(m_p2.getSymbol(), input))
				{
					user_input_iter++;
					continue;
				}
				break;
			}
			got_move = 1;
			pthread_join(timer_thread, NULL);

			// check if win
			if (m_board.isWin(input))
			{
				if (p1turn)
					cout << "Player 1 wins" << endl;
				else
					cout << "Player 2 wins" << endl;
				m_board.draw();
				c.send_win(input);
				exit(0);
			}
			// check if tie
			else if (m_board.isTie())
			{
				cout << "Tie game" << endl;
				m_board.draw();
				c.send_tie(input);
				exit(0);
			}
			else
			{
				if (!c.send_position(input))
				{
					cout << "error with send_position" << endl;
					return;
				}
			}
		}
		// wait for other player to make move
		else
		{
			pthread_t rcv_timer_thread;
			int got_move = 0;

			struct timer_params rcv_params_timer;
			rcv_params_timer.seconds = 45;
			rcv_params_timer.got_move = &got_move;
			rcv_params_timer.msg =
			"A move has not been received in 45 seconds. Closing connection.";
			rcv_params_timer.giveup = false;
			rcv_params_timer.giveup_t = &giveup_t;

			pthread_create(&rcv_timer_thread, NULL, &(Game::timer_countdown),
			&rcv_params_timer);
			pthread_mutex_lock(&Game::recv_buf_mutex);
			memset(m_recv_buf, 0, MAXBUFLEN);
			pthread_mutex_unlock(&Game::recv_buf_mutex);
			input = -1;
			while (got_move == 0)
			{
				pthread_mutex_lock(&Game::recv_buf_mutex);
				if (m_recv_buf[0] == 0)
				{
					pthread_mutex_unlock(&Game::recv_buf_mutex);
					input = -1;
					continue;
				}
				if (isdigit(m_recv_buf[0]) && m_recv_buf[0] != '0')
				{
					input = m_recv_buf[0] - '0';
					pthread_mutex_unlock(&Game::recv_buf_mutex);
					break;
				}
				pthread_mutex_unlock(&Game::recv_buf_mutex);
				input = -1;
			}
			got_move = 1;
			pthread_join(rcv_timer_thread, NULL);

			if (input == -1)
			{
				if (rcv_params_timer.giveup)
					c.send_giveup();
				else
					c.send_bye();
				exit(0);
			}

			if (p1turn && !m_board.insert(m_p1.getSymbol(), input))
			{
				cout << "error with receive_position with input: " << input
				<< endl;
				exit(1);
			}
			if (!p1turn && !m_board.insert(m_p2.getSymbol(), input))
			{
				cout << "error with receive_position with input: " << input << endl;
				exit(1);
			}
		}
		p1turn = !p1turn;
	}
}
