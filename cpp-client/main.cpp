#define VAR_SIZE 80

#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include "game.h"
using namespace std;

int main()
{
	// login
	char c_username[VAR_SIZE];
	char c_password[VAR_SIZE];

	memset(c_username, 0, VAR_SIZE);
	memset(c_password, 0, VAR_SIZE);

	cout << "Enter username (max 79 characters): ";
	cin >> c_username;
	c_username[VAR_SIZE - 1] = 0;
	strcpy(c_password, getpass("Enter password (max 79 characters): "));
	c_password[VAR_SIZE - 1] = 0;

	string username = c_username;
	string password = c_password;

	Game game;
	game.start(username, password);

	return 0;
}
