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
		@Override
		public void run() {
			for (;;) {
				if (TicTacToe.NotInGame.get())
					return;
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
					if (_recv.recvBuf != "" &&
							(_recv.recvBuf.charAt(0) == 'w' ||
							_recv.recvBuf.charAt(0) == 't') &&
							!TicTacToe.NotInGame.get()) {
						if (_recv.recvBuf.charAt(0) == 'w' ||
								_recv.recvBuf.charAt(0) == 't') {
							if (_c.isP1()) {
								_board.insert(Player.P2_SYMBOL,
										_recv.recvBuf.charAt(1) - '0');
								_ttt.getDisplay().doRepaint();
								if (_recv.recvBuf.charAt(0) == 'w') {
									JOptionPane.showMessageDialog(null, "Game over. You lose.");
									_ttt.getDisplay().GameOverMsgLock.lock();
									try {
										_ttt.getDisplay().GameOverMsg = "Player 2 wins.";
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
							else {
								_board.insert(Player.P1_SYMBOL,
										_recv.recvBuf.charAt(1) - '0');
								_ttt.getDisplay().doRepaint();
								if (_recv.recvBuf.charAt(0) == 'w') {
									JOptionPane.showMessageDialog(null, "Game over. You lose.");
									_ttt.getDisplay().GameOverMsgLock.lock();
									try {
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
							TicTacToe.NotInGame.set(true);
							_ttt.getDisplay().doRepaint();
							return;
						}
						TicTacToe.NotInGame.set(true);
						return;
					}
					if (_recv.recvBuf.equals("giveup"))
						break;
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

			int input;
			// insert at position
			if ((p1turn && _c.isP1()) || (!p1turn && !_c.isP1())) {
				final TimerThread.Msg msg = new TimerThread.Msg();
				msg.gotMsg = false;
				String errorMsg =
					"You have not played a move in 30 seconds. You have given up.";
				Runnable timer = new TimerThread(msg, 30, errorMsg, null,
						_ttt.getTimerfield());
				Thread t = new Thread(timer);
				t.start();

				input = -1;
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


				// check if win
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

					TicTacToe.NotInGame.set(true);
					_ttt.getDisplay().doRepaint();
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
					_ttt.getDisplay().doRepaint();

					_c.sendBye();
					return;
				}
				// check if tie
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

					_ttt.getDisplay().doRepaint();
					_ttt.getDisplay().GameOverMsgLock.lock();
					try {
						_ttt.getDisplay().GameOverMsg = "Tie game.";
					} finally {
						_ttt.getDisplay().GameOverMsgLock.unlock();
					}
					_c.sendBye();
					return;
				}
				else {
					if (input == -1) {
						TicTacToe.NotInGame.set(true);
						try {
							gt.join();
						} catch (InterruptedException e) {
							System.err.println("Could not join giveup thread.");
							System.exit(1);
						}
						_ttt.getDisplay().GameOverMsgLock.lock();
						try {
							if (TicTacToe.NotInGame.get() &&
									_ttt.getDisplay().GameOverMsg != null &&
									_ttt.getDisplay().GameOverMsg.equals("Click to start.")) {
								// quitbutton was triggered.
								if (_c.isP1())
									_ttt.getDisplay().GameOverMsg =
											"You have given up. Player 2 wins.";
								else
									_ttt.getDisplay().GameOverMsg =
											"You have given up. Player 1 wins.";
							}
							else if (TicTacToe.NotInGame.get() &&
									_ttt.getDisplay().GameOverMsg != null &&
									_ttt.getDisplay().GameOverMsg.contains("You win"))
								// other client triggered quitbutton
								;
							else if (TicTacToe.NotInGame.get() &&
									_ttt.getDisplay().GameOverMsg != null &&
									_ttt.getDisplay().GameOverMsg.equals("disconnect")) {
								// server disconnect
								_ttt.getDisplay().GameOverMsg = "Connection loss.";
								_c.sendBye();
							}
							else {
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
			}
			// wait for other player to make move
			else {
				final TimerThread.Msg msg = new TimerThread.Msg();
				msg.gotMsg = false;
				String errorMsg =
					"A move has not been received in 45 seconds. Closing connection.";
				Runnable timer = new TimerThread(msg, 45, errorMsg,
						_ttt.getDisplay(), null);
				Thread t = new Thread(timer);
				t.start();

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
						_ttt.getDisplay().GameOverMsgLock.lock();
						try {
							if (_ttt.getDisplay().GameOverMsg != null)
								break;
						} finally {
							_ttt.getDisplay().GameOverMsgLock.unlock();
						}
					} finally {
						_recv.recvBufLock.unlock();
					}
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

				msg.gotMsg = true;
				try {
					t.join();
				} catch (InterruptedException e) {
					System.err.println("Could not join timer thread.");
					System.exit(1);
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
			p1turn = !p1turn;
		}
	}

}
