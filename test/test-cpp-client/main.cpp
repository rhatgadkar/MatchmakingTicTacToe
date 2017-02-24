#define VAR_SIZE 21

#include <iostream>
#include "game.h"
using namespace std;

// format: ./a.out 1 2 3

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		cout << "Input arguments must be specified." << endl;
		return 1;
	}
	// each string in argv is from 1 - 9.
	int login_pos = -1;
	int* user_input = new int[argc - 1];
	for (int iter = 1; iter < argc; iter++)
	{
		char in = argv[iter][0];
		if (in < '1' || in > '9')
		{
			login_pos = iter;
			break;
		}
		user_input[iter - 1] = in - '0';
	}

	Game game;
	if (login_pos == -1)
		// no login
		game.start("", "", user_input, argc - 1);
	else
	{
		string login = argv[login_pos];
		game.start(login, login, user_input, login_pos - 1);
	}
	delete [] user_input;

	return 0;
}
