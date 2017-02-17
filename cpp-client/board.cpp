#include "board.h"
#include <iostream>
using namespace std;

void Board::draw() const
{
	for (int r = 0; r < ROWS; r++)
	{
		for (int c = 0; c < COLS; c++)
			cout << m_grid[r][c];
		cout << endl;
	}
}

void Board::clear()
{
	for (int r = 0; r < ROWS; r++)
		for (int c = 0; c < COLS; c++)
			m_grid[r][c] = '.';
}

const char* Board::m_getPos(int pos) const
{
	switch (pos)
	{
	case 1:
		return &(m_grid[0][0]);
	case 2:
		return &(m_grid[0][1]);
	case 3:
		return &(m_grid[0][2]);
	case 4:
		return &(m_grid[1][0]);
	case 5:
		return &(m_grid[1][1]);
	case 6:
		return &(m_grid[1][2]);
	case 7:
		return &(m_grid[2][0]);
	case 8:
		return &(m_grid[2][1]);
	case 9:
		return &(m_grid[2][2]);
	}
	return NULL;
}

bool Board::insert(char c, int pos)
{
	char* loc = (char*)m_getPos(pos);
	if (loc == NULL || *loc != '.')
		return false;
	*loc = c;
	return true;
}

bool Board::isWin(int pos) const
{
	const char* curr = m_getPos(pos);
	if (curr == NULL)
		return false;

	// check vertical
	const char* top2 = m_getPos(pos - 6);
	const char* top1 = m_getPos(pos - 3);
	const char* bot1 = m_getPos(pos + 3);
	const char* bot2 = m_getPos(pos + 6);

	if (top2 != NULL && top1 != NULL && *top2 == *curr && *top1 == *curr)
		return true;
	if (bot2 != NULL && bot1 != NULL && *bot2 == *curr && *bot1 == *curr)
		return true;
	if (top1 != NULL && bot1 != NULL && *top1 == *curr && *bot1 == *curr)
		return true;

	// check horizontal
	if (pos == 1 || pos == 4 || pos == 7)
	{
		const char* right1 = m_getPos(pos + 1);
		const char* right2 = m_getPos(pos + 2);
		if (right1 != NULL && right2 != NULL && *right1 == *curr &&
				*right2 == *curr)
			return true;
	}
	if (pos == 2 || pos == 5 || pos == 8)
	{
		const char* right1 = m_getPos(pos + 1);
		const char* left1 = m_getPos(pos - 1);
		if (right1 != NULL && left1 != NULL && *right1 == *curr &&
				*left1 == *curr)
			return true;
	}
	if (pos == 3 || pos == 6 || pos == 9)
	{
		const char* left1 = m_getPos(pos - 1);
		const char* left2 = m_getPos(pos - 2);
		if (left1 != NULL && left2 != NULL && *left1 == *curr &&
				*left2 == *curr)
			return true;
	}
	
	// check diagonal
	if (pos == 5 || pos == 1 || pos == 9 || pos == 3 || pos == 7)
	{
		const char* pos1 = m_getPos(1);
		const char* pos9 = m_getPos(9);
		const char* pos3 = m_getPos(3);
		const char* pos7 = m_getPos(7);
		const char* pos5 = m_getPos(5);

		if (pos == 5)
		{
			if (pos1 != NULL && pos9 != NULL && *pos1 == *curr &&
					*pos9 == *curr)
				return true;
			if (pos3 != NULL && pos7 != NULL && *pos3 == *curr &&
					*pos7 == *curr)
				return true;
		}
		if (pos == 1)
		{
			if (pos5 != NULL && pos9 != NULL && *pos5 == *curr &&
					*pos9 == *curr)
				return true;
		}
		if (pos == 9)
		{
			if (pos5 != NULL && pos1 != NULL && *pos5 == *curr &&
					*pos1 == *curr)
				return true;
		}
		if (pos == 3)
		{
			if (pos5 != NULL && pos7 != NULL && *pos5 == *curr &&
					*pos7 == *curr)
				return true;
		}
		if (pos == 7)
		{
			if (pos5 != NULL && pos3 != NULL && *pos5 == *curr &&
					*pos3 == *curr)
				return true;
		}
	}

	return false;
}

bool Board::isTie() const
{
	for (int r = 0; r < ROWS; r++)
	{
		for (int c = 0; c < COLS; c++)
		{
			if (m_grid[r][c] == '.')
				return false;
		}
	}
	return true;
}
