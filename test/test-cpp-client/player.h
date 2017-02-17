#ifndef PLAYER_H
#define PLAYER_H

class Player
{
public:
	Player(char symbol) { m_symbol = symbol; }
	~Player() {}
	char getSymbol() const { return m_symbol; }
	static const char P1_SYMBOL = 'x';
	static const char P2_SYMBOL = 'o';
private:
	char m_symbol;
};

#endif
