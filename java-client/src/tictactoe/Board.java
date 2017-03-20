package tictactoe;

// Board positions
// 123
// 456
// 789

// Empty board:
// ...
// ...
// ...

public final class Board {
	public static final int ROWS = 3;
	public static final int COLS = 3;
	private class Coordinate {
		char symbol;
	}
	private Coordinate _grid[][];

	public Board() {
		_grid = new Coordinate[Board.ROWS][Board.COLS];
		clear();
	}

	public char getSymbolAtCoord(int row, int col) {
		if (row >= Board.ROWS || row < 0 || col >= Board.COLS || col < 0)
			return 0;
		return _grid[row][col].symbol;
	}

	public void clear() {
		for (int r = 0; r < Board.ROWS; r++) {
			for (int c = 0; c < Board.COLS; c++) {
				_grid[r][c] = new Coordinate();
				_grid[r][c].symbol = '.';
			}
		}
	}

	public boolean insert(char c, int pos) {
		Coordinate loc = getPos(pos);
		if (loc == null || loc.symbol != '.')
			return false;
		loc.symbol = c;
		return true;
	}

	public boolean isWin(int pos) {
		final Coordinate curr = getPos(pos);
		if (curr == null)
			return false;

		// check vertical
		final Coordinate top2 = getPos(pos - 6);
		final Coordinate top1 = getPos(pos - 3);
		final Coordinate bot1 = getPos(pos + 3);
		final Coordinate bot2 = getPos(pos + 6);

		if (top2 != null && top1 != null && top2.symbol == curr.symbol &&
				top1.symbol == curr.symbol)
			return true;
		if (bot2 != null && bot1 != null && bot2.symbol == curr.symbol &&
				bot1.symbol == curr.symbol)
			return true;
		if (top1 != null && bot1 != null && top1.symbol == curr.symbol &&
				bot1.symbol == curr.symbol)
			return true;

		// check horizontal
		if (pos == 1 || pos == 4 || pos == 7) {
			final Coordinate right1 = getPos(pos + 1);
			final Coordinate right2 = getPos(pos + 2);
			if (right1 != null && right2 != null &&
					right1.symbol == curr.symbol &&
					right2.symbol == curr.symbol)
				return true;
		}
		if (pos == 2 || pos == 5 || pos == 8) {
			final Coordinate right1 = getPos(pos + 1);
			final Coordinate left1 = getPos(pos - 1);
			if (right1 != null && left1 != null &&
					right1.symbol == curr.symbol &&
					left1.symbol == curr.symbol)
				return true;
		}
		if (pos == 3 || pos == 6 || pos == 9) {
			final Coordinate left1 = getPos(pos - 1);
			final Coordinate left2 = getPos(pos - 2);
			if (left1 != null && left2 != null && left1.symbol == curr.symbol
					&& left2.symbol == curr.symbol)
				return true;
		}

		// check diagonal
		if (pos == 5 || pos == 1 || pos == 9 || pos == 3 || pos == 7) {
			final Coordinate pos1 = getPos(1);
			final Coordinate pos9 = getPos(9);
			final Coordinate pos3 = getPos(3);
			final Coordinate pos7 = getPos(7);
			final Coordinate pos5 = getPos(5);

			if (pos == 5) {
				if (pos1 != null && pos9 != null && pos1.symbol == curr.symbol
						&& pos9.symbol == curr.symbol)
					return true;
				if (pos3 != null && pos7 != null && pos3.symbol == curr.symbol
						&& pos7.symbol == curr.symbol)
					return true;
			}
			if (pos == 1) {
				if (pos5 != null && pos9 != null && pos5.symbol == curr.symbol
						&& pos9.symbol == curr.symbol)
					return true;
			}
			if (pos == 9) {
				if (pos5 != null && pos1 != null && pos5.symbol == curr.symbol
						&& pos1.symbol == curr.symbol)
					return true;
			}
			if (pos == 3) {
				if (pos5 != null && pos7 != null && pos5.symbol == curr.symbol
						&& pos7.symbol == curr.symbol)
					return true;
			}
			if (pos == 7) {
				if (pos5 != null && pos3 != null && pos5.symbol == curr.symbol
						&& pos3.symbol == curr.symbol)
					return true;
			}
		}

		return false;
	}

	private Coordinate getPos(int pos) {
		switch (pos) {
			case 1:
				return _grid[0][0];
			case 2:
				return _grid[0][1];
			case 3:
				return _grid[0][2];
			case 4:
				return _grid[1][0];
			case 5:
				return _grid[1][1];
			case 6:
				return _grid[1][2];
			case 7:
				return _grid[2][0];
			case 8:
				return _grid[2][1];
			case 9:
				return _grid[2][2];
		}
		return null;
	}

	public boolean isTie() {
		for (int r = 0; r < Board.ROWS; r++) {
			for (int c = 0; c < Board.COLS; c++) {
				if (_grid[r][c].symbol == '.')
					return false;
			}
		}
		return true;
	}
}
