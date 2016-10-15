#ifndef PLAYER_H
#define PLAYER_H

class Player
{
public:
	Player(char symbol) { m_symbol = symbol; }
	~Player() {}
	char getSymbol() const { return m_symbol; }

private:
	char m_symbol;
};

#endif
