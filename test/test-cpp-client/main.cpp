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
	int* user_input = new int[argc - 1];
	for (int iter = 1; iter < argc; iter++)
	{
		char in = argv[iter][0];
		if (in < '1' || in > '9')
		{
			cout << "Invalid user input. Aborting." << endl;
			return 1;
		}
		user_input[iter - 1] = in - '0';
	}

	Game game;
	// no login
	game.start("", "", user_input, argc - 1);
	delete [] user_input;

	return 0;
}
