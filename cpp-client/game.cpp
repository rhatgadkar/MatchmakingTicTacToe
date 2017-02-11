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
using namespace std;

Game::Game()
	: m_p1(Player::P1_SYMBOL), m_p2(Player::P2_SYMBOL)
{
	memset(m_recv_buf, 0, MAXBUFLEN);
}

void* Game::check_giveup(void* parameters)
{
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	
	struct check_giveup_params* params;
	params = (struct check_giveup_params*)parameters;

	do
	{
		params->c->receive_from_server(params->recv_buf);
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
				}
				else
				{
					params->board->insert(Player::P1_SYMBOL, params->recv_buf[1] - '0');
					if (params->recv_buf[0] == 'w')
						cout << "Player 1 wins" << endl;
					else
						cout << "Tie game" << endl;
				}
			}
			params->board->draw();
			cout << "Client is exiting.  Closing server." << endl;
			exit(0);
		}
	} while(strcmp(params->recv_buf, "giveup") != 0);

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
	*(params->got_move) = 1;
/*	if (params->msg == "You have not played a move in 30 seconds. You have given up.")
	{
		params->c->send_giveup();
		cout << "You giveup. You lose." << endl;
		exit(0);
	}*/
	if (params->giveup)
		params->c->send_giveup();
	else
		params->c->send_bye();
	exit(0);
	return NULL;
}

void Game::start(string username, string password)
{
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
	params.recv_buf = m_recv_buf;
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

			pthread_create(&timer_thread, NULL, &(Game::timer_countdown),
			&params_timer);
			while (got_move == 0)
			{
				input = -1;
				cout << "Enter position (1-9): ";
				string input_str;
				getline(cin, input_str);
				if (input_str.length() > 1 || input_str.length() < 1)
					continue;
				input = input_str[0] - '0';

				if (p1turn && !m_board.insert(m_p1.getSymbol(), input))
					continue;
				if (!p1turn && !m_board.insert(m_p2.getSymbol(), input))
					continue;
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
				pthread_cancel(giveup_t);
				c.send_win(input);
/*				char buf[MAXBUFLEN];
				memset(buf, 0, MAXBUFLEN);
				c.receive_from(buf, 1);
				if (buf[0] == 0 || strcmp(buf, "ACK") != 0)
					cout << "Lost connection. Not known if win got sent." << endl;*/
				c.send_bye();
				exit(0);
			}
			// check if tie
			else if (m_board.isTie())
			{
				cout << "Tie game" << endl;
				m_board.draw();
				c.send_tie(input);
				c.send_bye();
				exit(0);
			}
			// check if giveup
			else if (m_board.m_getPos(input) == NULL)
			{
				c.send_giveup();
				cout << "You giveup. You lose." << endl;
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

			pthread_create(&rcv_timer_thread, NULL, &(Game::timer_countdown),
			&rcv_params_timer);
			while (got_move == 0)
			{
				if (m_recv_buf[0] == 0)
				{
					input = -1;
					continue;
				}
				if (isdigit(m_recv_buf[0]) && m_recv_buf[0] != '0')
				{
					input = m_recv_buf[0] - '0';
					memset(m_recv_buf, 0, MAXBUFLEN);
					break;
				}
			}
			got_move = 1;
			pthread_join(rcv_timer_thread, NULL);
			
/*			// check if connection lost
			if (m_board.m_getPos(input) == NULL)
			{
				pthread_cancel(giveup_t);
				c.send_win(0);
				char buf[MAXBUFLEN];
				memset(buf, 0, MAXBUFLEN);
				c.receive_from(buf, 1);
				if (strcmp(buf, "ACK") == 0)
					cout << "Opponent lost connection. You win." << endl;
				else
					cout << "You lose. A connection loss could have occurred." << endl;
				c.send_bye();
				cout << "Connection loss." << endl;
				exit(0);
			}*/

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
