package tictactoe;

import java.net.SocketTimeoutException;
import java.util.concurrent.atomic.AtomicBoolean;

public class Game {
	private Board _board;
	private Player _p1;
	private Player _p2;
	private Recv _recv;
	private IClient _c;
	private ITicTacToe _ttt;
	
	public static AtomicBoolean NotInGame = new AtomicBoolean(true);
	
	private class Recv {
		private String _recvBuf;
		public synchronized void setRecvBuf(String newBuf) {
			_recvBuf = newBuf;
		}
		
		public synchronized String getRecvBuf() {
			return _recvBuf;
		}
	}
	
	public Game(ITicTacToe ttt, IClient c, Board b) {
		_ttt = ttt;
		_board = b;
		_c = c;
		_p1 = new Player(Player.P1_SYMBOL);
		_p2 = new Player(Player.P2_SYMBOL);
		_recv = new Recv();
		_recv.setRecvBuf("");
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
				if (isP1)
					_ttt.setGameOverMsg(ITicTacToe.P2_WIN_LOSE);
				else
					_ttt.setGameOverMsg(ITicTacToe.P1_WIN_LOSE);
			}
			else {
				_ttt.showGameOverDialog("Game over. Tie game.");
				_ttt.setGameOverMsg(ITicTacToe.TIE_GAME);
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
					_ttt.setGameOverMsg(ITicTacToe.DISCONNECT);
					Game.NotInGame.set(true);
					return;
				}
				synchronized (_recv) {
					_recv.setRecvBuf(Client.stringToLength(test, "giveup".length()));
					if (_recv.getRecvBuf().equals("giveup")) {
						Game.NotInGame.set(true);
						if (_c.isP1())
							_ttt.setGameOverMsg(ITicTacToe.P2_GIVEUP_WIN);
						else
							_ttt.setGameOverMsg(ITicTacToe.P1_GIVEUP_WIN);
						return;
					}
					if (_recv.getRecvBuf() == "")
						continue;
					if (_recv.getRecvBuf().charAt(0) == 'w' ||
							_recv.getRecvBuf().charAt(0) == 't') {
						handleRecvWinTie(_recv.getRecvBuf().charAt(0),
								_recv.getRecvBuf().charAt(1) - '0', _c.isP1());
						Game.NotInGame.set(true);
						return;
					}
				}
			}
		}
	}
	
	private void handleGameOver(boolean p1turn, boolean currPlayerTurn) {
		synchronized (_ttt) {
			if (_ttt.getGameOverMsg() != null &&
					_ttt.getGameOverMsg().equals(ITicTacToe.CLICK_TO_START)) {
				// quitbutton was triggered.
				_ttt.setGameOverMsg(ITicTacToe.GIVEN_UP);
				_c.sendGiveup();
			}
			else if (_ttt.getGameOverMsg() != null &&
					_ttt.getGameOverMsg().contains("given up"))
				// other client triggered quitbutton
				;
			else if (_ttt.getGameOverMsg() != null &&
					_ttt.getGameOverMsg().equals(ITicTacToe.DISCONNECT)) {
				// server disconnect
				_ttt.setGameOverMsg(ITicTacToe.CONNECTION_LOSS);
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
					_ttt.setGameOverMsg(ITicTacToe.NO_PLAY_MOVE);
					_c.sendGiveup();
				}
				else {
					_ttt.setGameOverMsg(ITicTacToe.CONNECTION_LOSS);
					_c.sendBye();
				}
			}
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

			_ttt.setGameOverMsg(ITicTacToe.YOU_WIN);
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

			_ttt.setGameOverMsg(ITicTacToe.TIE_GAME);
			return;
		}
		else if (input == -1) {
			// action happened before user could provide input or
			// user not input move within 30 seconds
			Game.NotInGame.set(true);
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
		_recv.setRecvBuf("");
		
		while (!Game.NotInGame.get()) {
			if (_recv.getRecvBuf() == "")
				continue;
			if (Character.isDigit(_recv.getRecvBuf().charAt(0)) &&
					_recv.getRecvBuf().charAt(0) != '0')
				break;
		}
		
		msg.gotMsg = true;
		try {
			t.join();
		} catch (InterruptedException e) {
			System.err.println("Could not join timer thread.");
			System.exit(1);
		}
		
		if (Game.NotInGame.get())
			return;

		input = _recv.getRecvBuf().charAt(0) - '0';

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
		boolean currPlayerTurn = false;
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
			
			currPlayerTurn = (p1turn && _c.isP1()) || (!p1turn && !_c.isP1());
			if (currPlayerTurn)
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
		
		handleGameOver(p1turn, currPlayerTurn);
	}
}
