package tictactoe;

import java.net.SocketTimeoutException;
import java.util.concurrent.locks.ReentrantLock;

import javax.swing.JOptionPane;

public class Game {
	private Board _board;
	private Player _p1;
	private Player _p2;
	private Recv _recv;
	private Client _c;
	private TicTacToe _ttt;
	
	private class Recv {
		public String recvBuf;
		public ReentrantLock recvBufLock = new ReentrantLock();
	}
	
	public Game(TicTacToe ttt, Client c, Board b) {
		_ttt = ttt;
		_board = b;
		_c = c;
		_p1 = new Player(Player.P1_SYMBOL);
		_p2 = new Player(Player.P2_SYMBOL);
		_recv = new Recv();
		_recv.recvBufLock.lock();
		_recv.recvBuf = "";
		_recv.recvBufLock.unlock();
	}
	
	public Board getBoard() {
		return _board;
	}
	
	private class CheckGiveupThread implements Runnable {
		private void handleRecvWinTie(char symbol, int pos, boolean isP1) {
			if (isP1)
				_board.insert(Player.P2_SYMBOL, pos);
			else
				_board.insert(Player.P1_SYMBOL, pos);
			_ttt.getDisplay().doRepaint();
			if (symbol == 'w') {
				JOptionPane.showMessageDialog(null, "Game over. You lose.");
				_ttt.getDisplay().GameOverMsgLock.lock();
				try {
					if (isP1)
						_ttt.getDisplay().GameOverMsg = "Player 2 wins.";
					else
						_ttt.getDisplay().GameOverMsg = "Player 1 wins.";
				} finally {
					_ttt.getDisplay().GameOverMsgLock.unlock();
				}
			}
			else {
				JOptionPane.showMessageDialog(null, "Game over. Tie game.");
				_ttt.getDisplay().GameOverMsgLock.lock();
				try {
					_ttt.getDisplay().GameOverMsg = "Tie game.";
				} finally {
					_ttt.getDisplay().GameOverMsgLock.unlock();
				}
			}
		}
		
		@Override
		public void run() {
			while (!TicTacToe.NotInGame.get()) {
				String test = "";
				try {
					test = _c.receiveFrom(1);
				} catch (SocketTimeoutException e) {
					continue;
				} catch (Exception e) {
					// server disconnect
					_ttt.getDisplay().GameOverMsgLock.lock();
					try {
						_ttt.getDisplay().GameOverMsg = "disconnect";
					} finally {
						_ttt.getDisplay().GameOverMsgLock.unlock();
					}
					TicTacToe.NotInGame.set(true);
					return;
				}
				_recv.recvBufLock.lock();
				try {
					_recv.recvBuf = TicTacToe.stringToLength(test, "giveup".length());
					if (_recv.recvBuf.equals("giveup"))
						break;
					if (_recv.recvBuf == "")
						continue;
					if (_recv.recvBuf.charAt(0) == 'w' ||
							_recv.recvBuf.charAt(0) == 't') {
						handleRecvWinTie(_recv.recvBuf.charAt(0),
								_recv.recvBuf.charAt(1) - '0', _c.isP1());
						TicTacToe.NotInGame.set(true);
						return;
					}
				} finally {
					_recv.recvBufLock.unlock();
				}
			}
			if (!TicTacToe.NotInGame.get()) {
				_ttt.getDisplay().GameOverMsgLock.lock();
				try {
					TicTacToe.NotInGame.set(true);
					if (_c.isP1())
						_ttt.getDisplay().GameOverMsg =
								"Player 2 has given up. You win.";
					else
						_ttt.getDisplay().GameOverMsg =
								"Player 1 has given up. You win.";
				} finally {
					_ttt.getDisplay().GameOverMsgLock.unlock();
				}
				return;
			}
		}
	}
	
	private void currPlayerMove(boolean p1turn, Thread gt) {
		final TimerThread.Msg msg = new TimerThread.Msg();
		msg.gotMsg = false;
		String errorMsg =
			"You have not played a move in 30 seconds. You have given up.";
		Runnable timer = new TimerThread(msg, 30, errorMsg, null,
				_ttt.getTimerfield());
		Thread t = new Thread(timer);
		t.start();

		int input = -1;
		if (p1turn) {
			input = _ttt.getDisplay().getInput(_p1.getSymbol());
		}
		if (!p1turn) {
			input = _ttt.getDisplay().getInput(_p2.getSymbol());
		}

		msg.gotMsg = true;
		try {
			t.join();
		} catch (InterruptedException e) {
			System.err.println("Could not join timer thread.");
			System.exit(1);
		}

		_ttt.setTimerfieldText("");

		if (_board.isWin(input) && !TicTacToe.NotInGame.get()) {
			_ttt.getDisplay().doRepaint();
			TicTacToe.NotInGame.set(true);
			try {
				gt.join();
			} catch (InterruptedException e) {
				System.err.println("Could not join giveup thread.");
				System.exit(1);
			}
			_c.sendWin(input);
			JOptionPane.showMessageDialog(null, "Game over. You win.");

			if (p1turn) {
				_ttt.getDisplay().GameOverMsgLock.lock();
				try {
					_ttt.getDisplay().GameOverMsg = "Player 1 wins.";
				} finally {
					_ttt.getDisplay().GameOverMsgLock.unlock();
				}
			}
			else {
				_ttt.getDisplay().GameOverMsgLock.lock();
				try {
					_ttt.getDisplay().GameOverMsg = "Player 2 wins.";
				} finally {
					_ttt.getDisplay().GameOverMsgLock.unlock();
				}
			}
			return;
		}
		else if (_board.isTie() && !TicTacToe.NotInGame.get()) {
			_ttt.getDisplay().doRepaint();
			TicTacToe.NotInGame.set(true);
			try {
				gt.join();
			} catch (InterruptedException e) {
				System.err.println("Could not join giveup thread.");
				System.exit(1);
			}
			_c.sendTie(input);
			JOptionPane.showMessageDialog(null, "Game over. Tie game.");

			_ttt.getDisplay().GameOverMsgLock.lock();
			try {
				_ttt.getDisplay().GameOverMsg = "Tie game.";
			} finally {
				_ttt.getDisplay().GameOverMsgLock.unlock();
			}
			return;
		}
		else if (input == -1) {
			// action happened before user could provide input or
			// user not input move within 30 seconds
			
			TicTacToe.NotInGame.set(true);
			try {
				gt.join();
			} catch (InterruptedException e) {
				System.err.println("Could not join giveup thread.");
				System.exit(1);
			}
			_ttt.getDisplay().GameOverMsgLock.lock();
			try {
				if (_ttt.getDisplay().GameOverMsg != null &&
						_ttt.getDisplay().GameOverMsg.equals("Click to start.")) {
					// quitbutton was triggered.
					if (_c.isP1())
						_ttt.getDisplay().GameOverMsg =
								"You have given up. Player 2 wins.";
					else
						_ttt.getDisplay().GameOverMsg =
								"You have given up. Player 1 wins.";
				}
				else if (_ttt.getDisplay().GameOverMsg != null &&
						_ttt.getDisplay().GameOverMsg.contains("You win"))
					// other client triggered quitbutton
					;
				else if (_ttt.getDisplay().GameOverMsg != null &&
						_ttt.getDisplay().GameOverMsg.equals("disconnect")) {
					// server disconnect
					_ttt.getDisplay().GameOverMsg = "Connection loss.";
					_c.sendBye();
				}
				else {
					// user didn't play move within 30 seconds
					if (p1turn)
						_ttt.getDisplay().GameOverMsg =
								"You have not played a move. Player 2 wins.";
					else
						_ttt.getDisplay().GameOverMsg =
								"You have not played a move. Player 1 wins.";
				}
			} finally {
				_ttt.getDisplay().GameOverMsgLock.unlock();
			}
			_c.sendGiveup();
			return;
		}
		else
			_c.sendPosition(input);
	}
	
	private void otherPlayerMove(boolean p1turn, Thread gt) {
		final TimerThread.Msg msg = new TimerThread.Msg();
		msg.gotMsg = false;
		String errorMsg =
			"A move has not been received in 45 seconds. Closing connection.";
		Runnable timer = new TimerThread(msg, 45, errorMsg,
				_ttt.getDisplay(), null);
		Thread t = new Thread(timer);
		t.start();

		int input = -1;
		_recv.recvBufLock.lock();
		try {
			_recv.recvBuf = "";
		} finally {
			_recv.recvBufLock.unlock();
		}
		
		while (!TicTacToe.NotInGame.get()) {
			_recv.recvBufLock.lock();
			try {
				if (_recv.recvBuf == "")
					continue;
				if (Character.isDigit(_recv.recvBuf.charAt(0)) &&
						_recv.recvBuf.charAt(0) != '0')
					break;
			} finally {
				_recv.recvBufLock.unlock();
			}
		}
		
		msg.gotMsg = true;
		try {
			t.join();
		} catch (InterruptedException e) {
			System.err.println("Could not join timer thread.");
			System.exit(1);
		}
		
		if (TicTacToe.NotInGame.get()) {
			try {
				gt.join();
			} catch (InterruptedException e) {
				System.err.println("Could not join giveup thread.");
				System.exit(1);
			}
			_ttt.getDisplay().GameOverMsgLock.lock();
			try {
				if (_ttt.getDisplay().GameOverMsg != null &&
						_ttt.getDisplay().GameOverMsg.equals("Click to start.")) {
					if (_c.isP1())
						_ttt.getDisplay().GameOverMsg =
								"You have given up. Player 2 wins.";
					else
						_ttt.getDisplay().GameOverMsg =
								"You have given up. Player 1 wins.";
					_c.sendGiveup();
				}
				else if (_ttt.getDisplay().GameOverMsg != null &&
						_ttt.getDisplay().GameOverMsg.equals("disconnect")) {
					// server disconnect
					_ttt.getDisplay().GameOverMsg = "Connection loss.";
					_c.sendBye();
				}
			} finally {
				_ttt.getDisplay().GameOverMsgLock.unlock();
			}
			return;
		}

		_recv.recvBufLock.lock();
		try {
			input = _recv.recvBuf.charAt(0) - '0';
		} finally {
			_recv.recvBufLock.unlock();
		}

		

		if (p1turn && !_board.insert(_p1.getSymbol(), input)) {
			System.err.println("Error with receivePosition with input: " +
					input);
			return;
		}
		if (!p1turn && !_board.insert(_p2.getSymbol(), input)) {
			System.err.println("Error with receivePosition with input: " +
					input);
			return;
		}
	}
	
	public void start(String username, String password) {
		if (TicTacToe.NotInGame.get()) {
			_ttt.setPlayerfieldText("MatchMaking TicTacToe");
			while (TicTacToe.NotInGame.get())
				;
			_ttt.getDisplay().doRepaint();
		}

		_ttt.setPlayerfieldText("Searching for opponent...");
		_c.init(username, password);

		if (!username.isEmpty() && !password.isEmpty()) {
			String[] initialSplit = _c.Record.split(",");
			String winRecord = initialSplit[0].split("r")[1];
			String lossRecord = initialSplit[1];
			_ttt.setWinfieldText("W: " + winRecord);
			_ttt.setLossfieldText("L: " + lossRecord);
		}

		if (_c.isP1())
			_ttt.setPlayerfieldText("You are player 1 (" +
					Player.P1_SYMBOL + ").");
		else
			_ttt.setPlayerfieldText("You are player 2 (" +
					Player.P2_SYMBOL + ").");

		Runnable giveupThread = new CheckGiveupThread();
		Thread gt = new Thread(giveupThread);
		gt.start();

		boolean p1turn = true;
		while (!TicTacToe.NotInGame.get()) {
			// draw board
			if (p1turn && _c.isP1())
				_ttt.setTurnfieldText("Your turn.");
			else if (p1turn && !_c.isP1())
				_ttt.setTurnfieldText("Player 1 turn.");
			else if (!p1turn && _c.isP1())
				_ttt.setTurnfieldText("Player 2 turn.");
			else
				_ttt.setTurnfieldText("Your turn.");

			_ttt.getDisplay().doRepaint();

			// insert at position
			if ((p1turn && _c.isP1()) || (!p1turn && !_c.isP1())) {
				currPlayerMove(p1turn, gt);
			}
			// wait for other player to make move
			else {
				otherPlayerMove(p1turn, gt);
			}
			p1turn = !p1turn;
		}
	}

}
