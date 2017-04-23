package tictactoe;

import java.net.SocketTimeoutException;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.locks.ReentrantLock;

public class Game {
	private Board _board;
	private Player _p1;
	private Player _p2;
	private Recv _recv;
	private IClient _c;
	private ITicTacToe _ttt;
	
	public static AtomicBoolean NotInGame = new AtomicBoolean(true);
	
	private class Recv {
		public String recvBuf;
		public ReentrantLock recvBufLock = new ReentrantLock();
	}
	
	public Game(ITicTacToe ttt, IClient c, Board b) {
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
			_ttt.repaintDisplay();
			if (symbol == 'w') {
				_ttt.showGameOverDialog("Game over. You lose.");
				_ttt.lockGameOverMsg();
				try {
					if (isP1)
						_ttt.setGameOverMsg("Player 2 wins. You lose.");
					else
						_ttt.setGameOverMsg("Player 1 wins. You lose.");
				} finally {
					_ttt.unlockGameOverMsg();
				}
			}
			else {
				_ttt.showGameOverDialog("Game over. Tie game.");
				_ttt.lockGameOverMsg();
				try {
					_ttt.setGameOverMsg("Tie game.");
				} finally {
					_ttt.unlockGameOverMsg();
				}
			}
		}
		
		@Override
		public void run() {
			/**
			 * Reads from the client socket and saves it into _recv.recvBuf.
			 * The data is a string sent from the other player.
			 * The game is over when:
			 * - the server is disconnected
			 * - a "giveup" is received from the other player, resulting in the
			 *   current player winning the game
			 * - a string beginning with 'w' or 't' is received from the other
			 *   player, signaling a loss for the current player or a tie game
			 * If the game is not over, reads from the client socket continue
			 * to get saved into _recv.recvBuf.
			 */
			while (!Game.NotInGame.get()) {
				String test = "";
				try {
					test = _c.receiveFrom(1);
				} catch (SocketTimeoutException e) {
					continue;
				} catch (Exception e) {
					// server disconnect
					_ttt.lockGameOverMsg();
					try {
						_ttt.setGameOverMsg("disconnect");
					} finally {
						_ttt.unlockGameOverMsg();
					}
					Game.NotInGame.set(true);
					return;
				}
				_recv.recvBufLock.lock();
				try {
					_recv.recvBuf = Client.stringToLength(test, "giveup".length());
					if (_recv.recvBuf.equals("giveup")) {
						_ttt.lockGameOverMsg();
						try {
							Game.NotInGame.set(true);
							if (_c.isP1())
								_ttt.setGameOverMsg(
										"Player 2 has given up. You win.");
							else
								_ttt.setGameOverMsg(
										"Player 1 has given up. You win.");
						} finally {
							_ttt.unlockGameOverMsg();
						}
						return;
					}
					if (_recv.recvBuf == "")
						continue;
					if (_recv.recvBuf.charAt(0) == 'w' ||
							_recv.recvBuf.charAt(0) == 't') {
						handleRecvWinTie(_recv.recvBuf.charAt(0),
								_recv.recvBuf.charAt(1) - '0', _c.isP1());
						Game.NotInGame.set(true);
						return;
					}
				} finally {
					_recv.recvBufLock.unlock();
				}
			}
		}
	}
	
	private void handleGameOver(boolean p1turn, boolean currPlayerTurn) {
		_ttt.lockGameOverMsg();
		try {
			if (_ttt.getGameOverMsg() != null &&
					_ttt.getGameOverMsg().equals("Click to start.")) {
				// quitbutton was triggered.
				_ttt.setGameOverMsg(
						"You have given up. You lose.");
				_c.sendGiveup();
			}
			else if (_ttt.getGameOverMsg() != null &&
					_ttt.getGameOverMsg().contains("given up"))
				// other client triggered quitbutton
				;
			else if (_ttt.getGameOverMsg() != null &&
					_ttt.getGameOverMsg().equals("disconnect")) {
				// server disconnect
				_ttt.setGameOverMsg("Connection loss.");
				_c.sendBye();
			}
			else if (_ttt.getGameOverMsg() != null &&
					(_ttt.getGameOverMsg().contains("You win") ||
					 _ttt.getGameOverMsg().contains("You lose") ||
					_ttt.getGameOverMsg().contains("Tie"))) {
				;
			}
			else {
				// user didn't play move within 30 seconds or
				// not receive move within 45 seconds
				if (currPlayerTurn) {
					_ttt.setGameOverMsg(
							"You have not played a move. You lose.");
					_c.sendGiveup();
				}
				else {
					_ttt.setGameOverMsg("Connection loss.");
					_c.sendBye();
				}
			}
		} finally {
			_ttt.unlockGameOverMsg();
		}
	}
	
	private void currPlayerMove(boolean p1turn, Thread gt) {
		/**
		 * Handle the event when it is the current player's move.  The current
		 * player has 30 seconds to play a move.  A current player's move is
		 * read from mouse input on a JPanel.
		 * The game is over when:
		 * - 30 seconds have expired, a "giveup" message will be sent to
		 *   the server, signaling the current player has given up
		 * - the current player pressed the "Quit" button, resulting in the
		 *   current player giving up and a "giveup" message sent to the server
		 * - the current player exited the program, resulting in the current
		 *   player giving up and a "giveup" message sent to the server
		 * - the other player gave up (determined by CheckGiveupThread)
		 * - the server is disconnected
		 * - the current player's move resulted in a win or tie game
		 * If the game is not over, the current player's move gets sent to the
		 * server and then it becomes the other player's move.
		 */
		final TimerThread.Msg msg = new TimerThread.Msg();
		msg.gotMsg = false;
		Runnable timer = new TimerThread(msg, 30, _ttt.getTimerfield());
		Thread t = new Thread(timer);
		t.start();

		int input = -1;
		if (p1turn) {
			input = _ttt.getInput(_p1.getSymbol());
		}
		if (!p1turn) {
			input = _ttt.getInput(_p2.getSymbol());
		}

		msg.gotMsg = true;
		try {
			t.join();
		} catch (InterruptedException e) {
			System.err.println("Could not join timer thread.");
			System.exit(1);
		}

		_ttt.setTimerfieldText("");

		if (_board.isWin(input) && !Game.NotInGame.get()) {
			_ttt.repaintDisplay();
			Game.NotInGame.set(true);
			try {
				gt.join();
			} catch (InterruptedException e) {
				System.err.println("Could not join giveup thread.");
				System.exit(1);
			}
			_c.sendWin(input);
			_ttt.showGameOverDialog("Game over. You win.");

			_ttt.lockGameOverMsg();
			try {
				_ttt.setGameOverMsg("You win.");
			} finally {
				_ttt.unlockGameOverMsg();
			}
			return;
		}
		else if (_board.isTie() && !Game.NotInGame.get()) {
			_ttt.repaintDisplay();
			Game.NotInGame.set(true);
			try {
				gt.join();
			} catch (InterruptedException e) {
				System.err.println("Could not join giveup thread.");
				System.exit(1);
			}
			_c.sendTie(input);
			_ttt.showGameOverDialog("Game over. Tie game.");

			_ttt.lockGameOverMsg();
			try {
				_ttt.setGameOverMsg("Tie game.");
			} finally {
				_ttt.unlockGameOverMsg();
			}
			return;
		}
		else if (input == -1) {
			// action happened before user could provide input or
			// user not input move within 30 seconds
			
			Game.NotInGame.set(true);
			try {
				gt.join();
			} catch (InterruptedException e) {
				System.err.println("Could not join giveup thread.");
				System.exit(1);
			}
			handleGameOver(p1turn, true);
			return;
		}
		else
			_c.sendPosition(input);
	}
	
	private void otherPlayerMove(boolean p1turn, Thread gt) {
		/**
		 * Handles the event when it is the other player's turn.  Up to 45
		 * seconds are spent waiting for a move from the other player.
		 * The game is over when:
		 * - the current player pressed the "Quit" button, resulting in the
		 *   current player giving up and a "giveup" message sent to the server
		 * - the current player exited the program, resulting in the current
		 *   player giving up and a "giveup" message sent to the server
		 * - the other player gave up (determined by CheckGiveupThread)
		 * - the server is disconnected
		 * - a move had not been received within 45 seconds, resulting in a
		 *   possible server disconnect from the other player, no win/loss
		 *   sent to server
		 * If the game is not over, the other player's move gets received from
		 * the server and then it becomes the current player's move. 
		 */
		final TimerThread.Msg msg = new TimerThread.Msg();
		msg.gotMsg = false;
		Runnable timer = new TimerThread(msg, 45, null);
		Thread t = new Thread(timer);
		t.start();

		int input = -1;
		_recv.recvBufLock.lock();
		try {
			_recv.recvBuf = "";
		} finally {
			_recv.recvBufLock.unlock();
		}
		
		while (!Game.NotInGame.get()) {
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
		
		if (Game.NotInGame.get()) {
			try {
				gt.join();
			} catch (InterruptedException e) {
				System.err.println("Could not join giveup thread.");
				System.exit(1);
			}
			handleGameOver(p1turn, false);
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
		if (Game.NotInGame.get()) {
			_ttt.setPlayerfieldText("MatchMaking TicTacToe");
			while (Game.NotInGame.get())
				;
			_ttt.repaintDisplay();
		}

		_ttt.setPlayerfieldText("Searching for opponent...");
		_c.init(username, password);

		/**
		 * record string is formatted like this:
		 * r[wins],[losses],[opponent name]
		*/
		String[] initialSplit = _c.getRecord().split(",");
		String winRecord = "";
		String lossRecord = "";
		String opponentUsername = "";
		try {
			winRecord = initialSplit[0].split("r")[1];
			lossRecord = initialSplit[1];
		} catch (Exception e) {
		}
		try {
			opponentUsername = initialSplit[2];
		} catch (Exception e) {
		}
		if (!username.isEmpty() && !password.isEmpty()) {
			_ttt.setWinfieldText("W: " + winRecord);
			_ttt.setLossfieldText("L: " + lossRecord);
		}
		if (PasswordWindow.isValidCredential(opponentUsername))
			_ttt.setOpponentText("Opponent: " + opponentUsername);
		else
			_ttt.setOpponentText("Guest Opponent");

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
		while (!Game.NotInGame.get()) {
			// draw board
			if (p1turn && _c.isP1())
				_ttt.setTurnfieldText("Your turn.");
			else if (p1turn && !_c.isP1())
				_ttt.setTurnfieldText("Player 1 turn.");
			else if (!p1turn && _c.isP1())
				_ttt.setTurnfieldText("Player 2 turn.");
			else
				_ttt.setTurnfieldText("Your turn.");

			_ttt.repaintDisplay();

			if ((p1turn && _c.isP1()) || (!p1turn && !_c.isP1()))
				currPlayerMove(p1turn, gt);
			else
				otherPlayerMove(p1turn, gt);
			p1turn = !p1turn;
		}
		
		try {
			gt.join();
		} catch (InterruptedException e) {
			System.err.println("Could not join giveup thread.");
			System.exit(1);
		}
	}
}
