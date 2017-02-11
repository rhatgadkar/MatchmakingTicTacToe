#define VAR_SIZE 21

#include <cstring>
#include <unistd.h>
#include <iostream>
#include <cctype>
#include <string>
#include "game.h"
using namespace std;

int main()
{
	// login
	string username;
	char c_password[VAR_SIZE];

	memset(c_password, 0, VAR_SIZE);

	cout << "Maximum username and password length is " << VAR_SIZE - 1 << " characters." << endl;
	cout << "Enter username: ";
	getline(cin, username);
	strcpy(c_password, getpass("Enter password: "));

	if (username.length() >= VAR_SIZE || c_password[VAR_SIZE - 1] != 0)
	{
		cout << "Username and password must be at most " << VAR_SIZE - 1 << " characters long." << endl;
		return 0;
	}

	string password = c_password;

	if (username.length() == 0 && password.length() == 0)
		;
	else
	{
		for (size_t k = 0; k < username.length(); k++)
		{
			if (!isalnum(username[k]))
			{
				cout << "Username and password must contain only letters and digits." << endl;
				return 0;
			}
		}
		for (size_t k = 0; k < password.length(); k++)
		{
			if (!isalnum(password[k]))
			{
				cout << "Username and password must contain only letters and digits." << endl;
				return 0;
			}
		}
	}

	Game game;
	game.start(username, password);

	return 0;
}
