package tests;

import org.junit.Assert;
import org.junit.Test;

import tictactoe.Board;
import tictactoe.Player;

public class TestBoard {

	@Test
	public void testBoard() {
		/**
		 * Verify Board can be successfully created and all the positions are
		 * initialized to '.'.
		 */
		Board board = new Board();
		for (int r = 0; r < Board.ROWS; r++) {
			for (int c = 0; c < Board.COLS; c++) {
				Assert.assertEquals('.', board.getSymbolAtCoord(r, c));
			}
		}
	}

	@Test
	public void testGetSymbolAtCoord() {
		/**
		 * Verify symbols in valid/invalid Board positions are successfully
		 * retrieved.  Position 5 is Player.P1_SYMBOL, position '2' is
		 * Player.P2_SYMBOL, and the rest are
		 * '.'.
		 */
		Board board = new Board();
		Assert.assertEquals(board.getSymbolAtCoord(Board.ROWS, 0), 0);
		Assert.assertEquals(board.getSymbolAtCoord(Board.ROWS + 1, 0), 0);
		Assert.assertEquals(board.getSymbolAtCoord(0, Board.COLS), 0);
		Assert.assertEquals(board.getSymbolAtCoord(0, Board.COLS + 1), 0);
		board.insert(Player.P1_SYMBOL, 5);
		board.insert(Player.P2_SYMBOL, 2);
		for (int r = 0; r < Board.ROWS; r++) {
			for (int c = 0; c < Board.COLS; c++) {
				char symbol = board.getSymbolAtCoord(r, c);
				// pos 5
				if (r == 1 && c == 1)
					Assert.assertEquals(symbol, Player.P1_SYMBOL);
				// pos 2
				else if (r == 0 && c == 1)
					Assert.assertEquals(symbol, Player.P2_SYMBOL);
				else
					Assert.assertEquals(symbol, '.');
			}
		}
	}

	@Test
	public void testClear() {
		/**
		 * Initialize pos 5 to Player.P1_SYMBOL and pos 2 to Player.P2_SYMBOL.
		 * After calling clear(), verify entire board is '.'.
		 */
		Board board = new Board();
		board.insert(Player.P1_SYMBOL, 5);
		board.insert(Player.P2_SYMBOL, 2);
		board.clear();
		for (int r = 0; r < Board.ROWS; r++) {
			for (int c = 0; c < Board.COLS; c++) {
				Assert.assertEquals(board.getSymbolAtCoord(r, c), '.');
			}
		}
	}

	@Test
	public void testInsert() {
		/**
		 * Initialize pos 5 to Player.P1_SYMBOL and pos 2 to Player.P2_SYMBOL.
		 * Verify pos 5 and pos 2 have Player.P1_SYMBOL and Player.P2_SYMBOL
		 * respectively. The rest of the positions should be '.'.
		 */
		Board board = new Board();
		board.insert(Player.P1_SYMBOL, 5);
		board.insert(Player.P2_SYMBOL, 2);
		for (int r = 0; r < Board.ROWS; r++) {
			for (int c = 0; c < Board.COLS; c++) {
				char symbol = board.getSymbolAtCoord(r, c);
				// pos 5
				if (r == 1 && c == 1)
					Assert.assertEquals(symbol, Player.P1_SYMBOL);
				// pos 2
				else if (r == 0 && c == 1)
					Assert.assertEquals(symbol, Player.P2_SYMBOL);
				else
					Assert.assertEquals(symbol, '.');
			}
		}
	}

	@Test
	public void testIsWin() {
		/**
		 * Test vertical, horizontal, and diagonal wins for Player.P1_SYMBOL and
		 * Player.P2_SYMBOL.
		 */
		Board board = new Board();
		// horizontal Player.P1_SYMBOL win
		board.insert(Player.P1_SYMBOL, 1);
		board.insert(Player.P1_SYMBOL, 2);
		board.insert(Player.P1_SYMBOL, 3);
		Assert.assertTrue(board.isWin(1));
		Assert.assertTrue(board.isWin(2));
		Assert.assertTrue(board.isWin(3));
		// vertical Player.P1_SYMBOL win
		board.insert(Player.P1_SYMBOL, 1);
		board.insert(Player.P1_SYMBOL, 4);
		board.insert(Player.P1_SYMBOL, 7);
		Assert.assertTrue(board.isWin(1));
		Assert.assertTrue(board.isWin(4));
		Assert.assertTrue(board.isWin(7));
		// diagonal Player.P1_SYMBOL win
		board.insert(Player.P1_SYMBOL, 1);
		board.insert(Player.P1_SYMBOL, 5);
		board.insert(Player.P1_SYMBOL, 9);
		Assert.assertTrue(board.isWin(1));
		Assert.assertTrue(board.isWin(5));
		Assert.assertTrue(board.isWin(9));
		// horizontal Player.P2_SYMBOL win
		board.insert(Player.P2_SYMBOL, 1);
		board.insert(Player.P2_SYMBOL, 2);
		board.insert(Player.P2_SYMBOL, 3);
		Assert.assertTrue(board.isWin(1));
		Assert.assertTrue(board.isWin(2));
		Assert.assertTrue(board.isWin(3));
		// vertical Player.P2_SYMBOL win
		board.insert(Player.P2_SYMBOL, 1);
		board.insert(Player.P2_SYMBOL, 4);
		board.insert(Player.P2_SYMBOL, 7);
		Assert.assertTrue(board.isWin(1));
		Assert.assertTrue(board.isWin(4));
		Assert.assertTrue(board.isWin(7));
		// diagonal Player.P2_SYMBOL win
		board.insert(Player.P2_SYMBOL, 1);
		board.insert(Player.P2_SYMBOL, 5);
		board.insert(Player.P2_SYMBOL, 9);
		Assert.assertTrue(board.isWin(1));
		Assert.assertTrue(board.isWin(5));
		Assert.assertTrue(board.isWin(9));
	}

	@Test
	public void testIsTie() {
		/**
		 * Test tie game.
		 */
		Board board = new Board();
		board.insert(Player.P1_SYMBOL, 1);
		board.insert(Player.P2_SYMBOL, 2);
		board.insert(Player.P1_SYMBOL, 3);
		board.insert(Player.P2_SYMBOL, 4);
		board.insert(Player.P1_SYMBOL, 6);
		board.insert(Player.P2_SYMBOL, 5);
		board.insert(Player.P1_SYMBOL, 8);
		board.insert(Player.P2_SYMBOL, 7);
		board.insert(Player.P2_SYMBOL, 9);
		Assert.assertTrue(board.isTie());
	}

}
