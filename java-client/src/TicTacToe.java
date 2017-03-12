import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.FlowLayout;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JLabel;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.locks.ReentrantLock;
import java.net.SocketTimeoutException;

@SuppressWarnings("serial")
public final class TicTacToe extends JPanel {

	public static AtomicBoolean NotInGame = new AtomicBoolean(true);

	public static String stringToLength(String input, int length) {
		StringBuilder sb = new StringBuilder(input);
		sb.setLength(length);
		return sb.toString();
	}

	private Board _board;
	private Player _p1;
	private Player _p2;
	private Recv _recv;
	private Display _display;
	private JLabel _turnfield;
	private JLabel _playerfield;
	private JLabel _timerfield;
	private JButton _quitbutton;
	private Client _c;
	private JLabel _winrecordfield;
	private JLabel _lossrecordfield;

	public Display getDisplay() {
		return _display;
	}

	private class Recv {
		public String recvBuf;
		public ReentrantLock recvBufLock = new ReentrantLock();
	}

	private class CheckGiveupThread implements Runnable {
		private final Recv _recv;
		private Client _c;
		private Board _board;
		private Display _display;
		public CheckGiveupThread(Recv recv, Client c, Board board,
				Display display) {
			_recv = recv;
			_c = c;
			_board = board;
			_display = display;
		}
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
					_display.GameOverMsgLock.lock();
					try {
						_display.GameOverMsg = "disconnect";
					} finally {
						_display.GameOverMsgLock.unlock();
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
								_display.doRepaint();
								if (_recv.recvBuf.charAt(0) == 'w') {
									JOptionPane.showMessageDialog(null, "Game over. You lose.");
									_display.GameOverMsgLock.lock();
									try {
										_display.GameOverMsg = "Player 2 wins.";
									} finally {
										_display.GameOverMsgLock.unlock();
									}
								}
								else {
									JOptionPane.showMessageDialog(null, "Game over. Tie game.");
									_display.GameOverMsgLock.lock();
									try {
										_display.GameOverMsg = "Tie game.";
									} finally {
										_display.GameOverMsgLock.unlock();
									}
								}
							}
							else {
								_board.insert(Player.P1_SYMBOL,
										_recv.recvBuf.charAt(1) - '0');
								_display.doRepaint();
								if (_recv.recvBuf.charAt(0) == 'w') {
									JOptionPane.showMessageDialog(null, "Game over. You lose.");
									_display.GameOverMsgLock.lock();
									try {
										_display.GameOverMsg = "Player 1 wins.";
									} finally {
										_display.GameOverMsgLock.unlock();
									}
								}
								else {
									JOptionPane.showMessageDialog(null, "Game over. Tie game.");
									_display.GameOverMsgLock.lock();
									try {
										_display.GameOverMsg = "Tie game.";
									} finally {
										_display.GameOverMsgLock.unlock();
									}
								}
							}
							TicTacToe.NotInGame.set(true);
							_display.doRepaint();
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
				_display.GameOverMsgLock.lock();
				try {
					TicTacToe.NotInGame.set(true);
					if (_c.isP1())
						_display.GameOverMsg =
								"Player 2 has given up. You win.";
					else
						_display.GameOverMsg =
								"Player 1 has given up. You win.";
				} finally {
					_display.GameOverMsgLock.unlock();
				}
				return;
			}
		}
	}

	public static void main(String[] args) {
		// login
		JFrame login = new JFrame("Login");
		login.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		PasswordWindow loginPrompt = new PasswordWindow();
		login.setContentPane(loginPrompt);
		login.pack();
		login.setVisible(true);
		while (loginPrompt.Username == null && loginPrompt.Password == null)
			;
		String username = new String(loginPrompt.Username);
		String password = new String(loginPrompt.Password);
		login.setVisible(false);
		login.dispose();

		JFrame window = new JFrame("Matchmaking TicTacToe");
		window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		while (true) {
			TicTacToe game = new TicTacToe();
			window.setContentPane(game);
			window.pack();
			window.setVisible(true);

			game.start(username, password);
			System.out.println("Exited game.start()");
			game.getDisplay().doRepaint();

			while (TicTacToe.NotInGame.get())
				;
		}
	}

	private JPanel createTopPanel() {
		JPanel p = new JPanel(new FlowLayout(FlowLayout.CENTER, 50, 0));
		p.add(_turnfield);
		p.add(_playerfield);
		p.add(_timerfield);
		return p;
	}

	private JPanel createRightGrid() {
		JPanel p = new JPanel(new GridLayout(3, 1));
		p.add(_quitbutton);
		p.add(_winrecordfield);
		p.add(_lossrecordfield);
		return p;
	}

	private JPanel createBotPanel() {
		JPanel p = new JPanel(new FlowLayout());
		p.add(_display);
		p.add(createRightGrid());
		return p;
	}

	public TicTacToe() {
		_board = new Board();
		_p1 = new Player(Player.P1_SYMBOL);
		_p2 = new Player(Player.P2_SYMBOL);
		_recv = new Recv();
		_recv.recvBufLock.lock();
		_recv.recvBuf = "";
		_recv.recvBufLock.unlock();
		_c = new Client();

		_display = new Display(_board);
		_display.setPreferredSize(new Dimension(Display.WIDTH,
				Display.HEIGHT));
		_turnfield = new JLabel();
		_playerfield = new JLabel();
		_timerfield = new JLabel();
		_quitbutton = new JButton("Quit");
		_timerfield.setText("");
		_winrecordfield = new JLabel();
		_lossrecordfield = new JLabel();

		_quitbutton.addActionListener(new ActionListener() {
			private Display _display;
			private Client _c;
			public void actionPerformed(ActionEvent e) {
				if (TicTacToe.NotInGame.get() || !_c.DoneInit)
					System.exit(0);
				else {
					_display.GameOverMsgLock.lock();
					try {
						TicTacToe.NotInGame.set(true);
						_display.GameOverMsg = "Click to start.";
					} finally {
						_display.GameOverMsgLock.unlock();
					}
				}
			}
			private ActionListener init(Display display, Client c) {
				_display = display;
				_c = c;
				return this;
			}
		}.init(_display, _c));

		setLayout(new GridLayout(2, 1));
		add(createTopPanel());
		add(createBotPanel());
	}

	public void start(String username, String password) {
		if (TicTacToe.NotInGame.get()) {
			_playerfield.setText("MatchMaking TicTacToe");
			while (TicTacToe.NotInGame.get())
				;
			_display.doRepaint();
		}

		_playerfield.setText("Searching for opponent...");
		_c.init(username, password);

		if (!username.isEmpty() && !password.isEmpty()) {
			String[] initialSplit = _c.Record.split(",");
			String winRecord = initialSplit[0].split("r")[1];
			String lossRecord = initialSplit[1];
			_winrecordfield.setText("W: " + winRecord);
			_lossrecordfield.setText("L: " + lossRecord);
		}

		if (_c.isP1())
			_playerfield.setText("You are player 1 (" +
					Player.P1_SYMBOL + ").");
		else
			_playerfield.setText("You are player 2 (" +
					Player.P2_SYMBOL + ").");

		Runnable giveupThread = new CheckGiveupThread(_recv, _c,
				_board, _display);
		Thread gt = new Thread(giveupThread);
		gt.start();

		boolean p1turn = true;
		while (!TicTacToe.NotInGame.get()) {
			// draw board
			if (p1turn && _c.isP1())
				_turnfield.setText("Your turn.");
			else if (p1turn && !_c.isP1())
				_turnfield.setText("Player 1 turn.");
			else if (!p1turn && _c.isP1())
				_turnfield.setText("Player 2 turn.");
			else
				_turnfield.setText("Your turn.");

			_display.doRepaint();

			int input;
			// insert at position
			if ((p1turn && _c.isP1()) || (!p1turn && !_c.isP1())) {
				final TimerThread.Msg msg = new TimerThread.Msg();
				msg.gotMsg = false;
				String errorMsg =
					"You have not played a move in 30 seconds. You have given up.";
				Runnable timer = new TimerThread(msg, 30, errorMsg, null,
						_timerfield);
				Thread t = new Thread(timer);
				t.start();

				input = -1;
				if (p1turn) {
					input = _display.getInput(_p1.getSymbol());
				}
				if (!p1turn) {
					input = _display.getInput(_p2.getSymbol());
				}

				msg.gotMsg = true;
				try {
					t.join();
				} catch (InterruptedException e) {
					System.err.println("Could not join timer thread.");
					System.exit(1);
				}

				_timerfield.setText("");


				// check if win
				if (_board.isWin(input) && !TicTacToe.NotInGame.get()) {
					_display.doRepaint();
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
					_display.doRepaint();
					if (p1turn) {
						_display.GameOverMsgLock.lock();
						try {
							_display.GameOverMsg = "Player 1 wins.";
						} finally {
							_display.GameOverMsgLock.unlock();
						}
					}
					else {
						_display.GameOverMsgLock.lock();
						try {
							_display.GameOverMsg = "Player 2 wins.";
						} finally {
							_display.GameOverMsgLock.unlock();
						}
					}
					_display.doRepaint();

					_c.sendBye();
					return;
				}
				// check if tie
				else if (_board.isTie() && !TicTacToe.NotInGame.get()) {
					_display.doRepaint();
					TicTacToe.NotInGame.set(true);
					try {
						gt.join();
					} catch (InterruptedException e) {
						System.err.println("Could not join giveup thread.");
						System.exit(1);
					}
					_c.sendTie(input);
					JOptionPane.showMessageDialog(null, "Game over. Tie game.");

					_display.doRepaint();
					_display.GameOverMsgLock.lock();
					try {
						_display.GameOverMsg = "Tie game.";
					} finally {
						_display.GameOverMsgLock.unlock();
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
						_display.GameOverMsgLock.lock();
						try {
							if (TicTacToe.NotInGame.get() &&
									_display.GameOverMsg != null &&
									_display.GameOverMsg.equals("Click to start.")) {
								// quitbutton was triggered.
								if (_c.isP1())
									_display.GameOverMsg =
											"You have given up. Player 2 wins.";
								else
									_display.GameOverMsg =
											"You have given up. Player 1 wins.";
							}
							else if (TicTacToe.NotInGame.get() &&
									_display.GameOverMsg != null &&
									_display.GameOverMsg.contains("You win"))
								// other client triggered quitbutton
								;
							else if (TicTacToe.NotInGame.get() &&
									_display.GameOverMsg != null &&
									_display.GameOverMsg.equals("disconnect")) {
								// server disconnect
								_display.GameOverMsg = "Connection loss.";
								_c.sendBye();
							}
							else {
								if (p1turn)
									_display.GameOverMsg =
											"You have not played a move. Player 2 wins.";
								else
									_display.GameOverMsg =
											"You have not played a move. Player 1 wins.";
							}
						} finally {
							_display.GameOverMsgLock.unlock();
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
						_display, null);
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
						_display.GameOverMsgLock.lock();
						try {
							if (_display.GameOverMsg != null)
								break;
						} finally {
							_display.GameOverMsgLock.unlock();
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
					_display.GameOverMsgLock.lock();
					try {
						if (_display.GameOverMsg != null &&
								_display.GameOverMsg.equals("Click to start.")) {
							if (_c.isP1())
								_display.GameOverMsg =
										"You have given up. Player 2 wins.";
							else
								_display.GameOverMsg =
										"You have given up. Player 1 wins.";
							_c.sendGiveup();
						}
						else if (_display.GameOverMsg != null &&
								_display.GameOverMsg.equals("disconnect")) {
							// server disconnect
							_display.GameOverMsg = "Connection loss.";
							_c.sendBye();
						}
					} finally {
						_display.GameOverMsgLock.unlock();
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
