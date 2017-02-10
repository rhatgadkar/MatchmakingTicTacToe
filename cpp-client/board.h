#ifndef BOARD_H
#define BOARD_H

// Board positions:
// 123
// 456
// 789
//
//empty Board
// ...
// ...
// ...

#define ROWS 3
#define COLS 3

class Board
{
public:
	Board() { clear(); }
	~Board() {}
	void draw() const;
	void clear();
	bool insert(char c, int pos);
	bool isWin(int pos) const;
	bool isTie() const;
	const char* m_getPos(int pos) const;
private:
	char m_grid[ROWS][COLS];
};

#endif
