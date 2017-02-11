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
import javax.swing.JOptionPane;

@SuppressWarnings("serial")
public final class TicTacToe extends JPanel {

	public static volatile boolean NotInGame = true;

	public static String stringToLength(String input, int length) {
		StringBuilder sb = new StringBuilder(input);
		sb.setLength(length);
		return sb.toString();
	}

	private Board board;
	private Player p1;
	private Player p2;
	private Recv recv;
	private Display display;
	private JLabel turnfield;
	private JLabel playerfield;
	private JLabel timerfield;
	private JButton quitbutton;
	private Client c;
	private JLabel winrecordfield;
	private JLabel lossrecordfield;

	public Display getDisplay() {
		return this.display;
	}

	private class Recv {
		public volatile String recvBuf;
	}

	private class CheckGiveupThread implements Runnable {
		private final Recv recv;
		private Client c;
		private Board board;
		private Display display;
		public CheckGiveupThread(Recv recv, Client c, Board board, Display display) {
			this.recv = recv;
			this.c = c;
			this.board = board;
			this.display = display;
		}
		@Override
		public void run() {
			do {
				if (TicTacToe.NotInGame) {
					return;
				}
				try {
					String test = this.c.receiveFrom(1);
					this.recv.recvBuf = TicTacToe.stringToLength(test, "giveup".length());
				} catch (DisconnectException e) {
					if (TicTacToe.NotInGame) {
						return;
					}
					TicTacToe.NotInGame = true;
					if (this.c.isP1())
						this.display.gameOverMsg = "Player 2 has given up. You win.";
					else
						this.display.gameOverMsg = "Player 1 has given up. You win.";
					return;
				} catch (Exception e) {
					continue;
				}
				if (this.recv.recvBuf != "" &&
						(this.recv.recvBuf.charAt(0) == 'w' || this.recv.recvBuf.charAt(0) == 't') &&
						!TicTacToe.NotInGame) {
					if (this.recv.recvBuf.charAt(0) == 'w' ||
							this.recv.recvBuf.charAt(0) == 't') {
						if (this.c.isP1()) {
							this.board.insert(Player.P2_SYMBOL, this.recv.recvBuf.charAt(1) - '0');
							this.display.doRepaint();
							if (this.recv.recvBuf.charAt(0) == 'w') {
								JOptionPane.showMessageDialog(null, "Game over. You lose.");
								this.display.gameOverMsg = "Player 2 wins.";
							}
							else {
								JOptionPane.showMessageDialog(null, "Game over. Tie game.");
								this.display.gameOverMsg = "Tie game.";
							}
						}
						else {
							this.board.insert(Player.P1_SYMBOL, this.recv.recvBuf.charAt(1) - '0');
							this.display.doRepaint();
							if (this.recv.recvBuf.charAt(0) == 'w') {
								JOptionPane.showMessageDialog(null, "Game over. You lose.");
								this.display.gameOverMsg = "Player 1 wins.";
							}
							else {
								JOptionPane.showMessageDialog(null, "Game over. Tie game.");
								this.display.gameOverMsg = "Tie game.";
							}
						}
						TicTacToe.NotInGame = true;
						this.display.doRepaint();
						return;
					}
					TicTacToe.NotInGame = true;
					return;
				}
			} while (!this.recv.recvBuf.equals("giveup"));
			if (!TicTacToe.NotInGame) {
				TicTacToe.NotInGame = true;
				if (this.c.isP1())
					this.display.gameOverMsg = "Player 2 has given up. You win.";
				else
					this.display.gameOverMsg = "Player 1 has given up. You win.";
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

			while (TicTacToe.NotInGame)
				;
		}
	}

	private JPanel createTopPanel() {
		JPanel p = new JPanel(new FlowLayout(FlowLayout.CENTER, 50, 0));
		p.add(this.turnfield);
		p.add(this.playerfield);
		p.add(this.timerfield);
		return p;
	}

	private JPanel createRightGrid() {
		JPanel p = new JPanel(new GridLayout(3, 1));
		p.add(this.quitbutton);
		p.add(this.winrecordfield);
		p.add(this.lossrecordfield);
		return p;
	}

	private JPanel createBotPanel() {
		JPanel p = new JPanel(new FlowLayout());
		p.add(this.display);
		p.add(createRightGrid());
		return p;
	}

	public TicTacToe() {
		this.board = new Board();
		this.p1 = new Player(Player.P1_SYMBOL);
		this.p2 = new Player(Player.P2_SYMBOL);
		this.recv = new Recv();
		this.recv.recvBuf = "";
		this.c = new Client();

		this.display = new Display(this.board);
		this.display.setPreferredSize(new Dimension(Display.WIDTH, Display.HEIGHT));
		this.turnfield = new JLabel();
		this.playerfield = new JLabel();
		this.timerfield = new JLabel();
		this.quitbutton = new JButton("Quit");
		this.timerfield.setText("");
		this.winrecordfield = new JLabel();
		this.lossrecordfield = new JLabel();

		this.quitbutton.addActionListener(new ActionListener() {
			private Display display;
			private Client c;
			public void actionPerformed(ActionEvent e) {
				if (TicTacToe.NotInGame || !this.c.DoneInit)
					System.exit(0);
				else {
					TicTacToe.NotInGame = true;
					this.display.gameOverMsg = "Click to start.";
				}
			}
			private ActionListener init(Display display, Client c) {
				this.display = display;
				this.c = c;
				return this;
			}
		}.init(this.display, this.c));

		setLayout(new GridLayout(2, 1));
		add(createTopPanel());
		add(createBotPanel());
	}

	public void start(String username, String password) {
		if (TicTacToe.NotInGame) {
			this.playerfield.setText("MatchMaking TicTacToe");
			while (TicTacToe.NotInGame)
				;
			this.display.doRepaint();
		}

		this.playerfield.setText("Searching for opponent...");
		this.c.init(username, password);

		if (!username.isEmpty() && !password.isEmpty()) {
			String[] initialSplit = this.c.Record.split(",");
			String winRecord = initialSplit[0].split("r")[1];
			String lossRecord = initialSplit[1];
			this.winrecordfield.setText("W: " + winRecord);
			this.lossrecordfield.setText("L: " + lossRecord);
		}

		if (this.c.isP1())
			this.playerfield.setText("You are player 1 (" + Player.P1_SYMBOL + ").");
		else
			this.playerfield.setText("You are player 2 (" + Player.P2_SYMBOL + ").");

		Runnable giveupThread = new CheckGiveupThread(this.recv, this.c, this.board, this.display);
		Thread gt = new Thread(giveupThread);
		gt.start();

		boolean p1turn = true;
		while (!TicTacToe.NotInGame) {
			// draw board
			if (p1turn && this.c.isP1())
				this.turnfield.setText("Your turn.");
			else if (p1turn && !this.c.isP1())
				this.turnfield.setText("Player 1 turn.");
			else if (!p1turn && this.c.isP1())
				this.turnfield.setText("Player 2 turn.");
			else
				this.turnfield.setText("Your turn.");

			this.display.doRepaint();

			int input;
			// insert at position
			if ((p1turn && this.c.isP1()) || (!p1turn && !this.c.isP1())) {
				final TimerThread.Msg msg = new TimerThread.Msg();
				msg.gotMsg = false;
				String errorMsg =
					"You have not played a move in 30 seconds. You have given up.";
				Runnable timer = new TimerThread(msg, 30, errorMsg, null, this.timerfield);
				Thread t = new Thread(timer);
				t.start();

				input = -1;
				if (p1turn) {
					input = this.display.getInput(this.p1.getSymbol());
				}
				if (!p1turn) {
					input = this.display.getInput(this.p2.getSymbol());
				}

				msg.gotMsg = true;
				try {
					t.join();
				} catch (InterruptedException e) {
					System.err.println("Could not join timer thread.");
					System.exit(1);
				}

				this.timerfield.setText("");


				// check if win
				if (this.board.isWin(input) && !TicTacToe.NotInGame) {
					this.display.doRepaint();
					TicTacToe.NotInGame = true;
					try {
						gt.join();
					} catch (InterruptedException e) {
						System.err.println("Could not join giveup thread.");
						System.exit(1);
					}
					c.sendWin(input);
/*					String ack = "";
					try {
						ack = this.c.receiveFrom(1);
					} catch (Exception e) {
						this.display.gameOverMsg = "Lost connection. Not known if win got sent.";
						this.display.doRepaint();
						return;
					}
					if (!ack.equals("ACK")) {
						this.display.gameOverMsg = "Lost connection. Not known if win got sent.";
						this.display.doRepaint();
						return;
					}*/
					JOptionPane.showMessageDialog(null, "Game over. You win.");

					if (p1turn)
						this.display.gameOverMsg = "Player 1 wins.";
					else
						this.display.gameOverMsg = "Player 2 wins.";
					this.display.doRepaint();

					this.c.sendBye();
					return;
				}
				// check if tie
				else if (this.board.isTie() && !TicTacToe.NotInGame) {
					this.display.doRepaint();
					c.sendTie(input);
					JOptionPane.showMessageDialog(null, "Game over. Tie game.");

					TicTacToe.NotInGame = true;
					this.display.doRepaint();
					this.display.gameOverMsg = "Tie game.";
					this.c.sendBye();
					return;
				}
				else {
					if (input == -1) {
						if (TicTacToe.NotInGame && this.display.gameOverMsg != null && this.display.gameOverMsg.equals("Click to start.")) {
							// quitbutton was triggered.
							if (this.c.isP1())
								this.display.gameOverMsg = "You have given up. Player 2 wins.";
							else
								this.display.gameOverMsg = "You have given up. Player 1 wins.";
						}
						else if (TicTacToe.NotInGame && this.display.gameOverMsg != null && this.display.gameOverMsg.contains("You win"))
							// other client triggered quitbutton - disconnectexception
							;
						else {
							if (p1turn)
								this.display.gameOverMsg = "You have not played a move. Player 2 wins.";
							else
								this.display.gameOverMsg = "You have not played a move. Player 1 wins.";
						}
						this.c.sendGiveup();
						return;
					}
					else
						this.c.sendPosition(input);
				}
			}
			// wait for other player to make move
			else {
				final TimerThread.Msg msg = new TimerThread.Msg();
				msg.gotMsg = false;
				String errorMsg =
					"A move has not been received in 45 seconds. Closing connection.";
				Runnable timer = new TimerThread(msg, 45, errorMsg, this.display, null);
				Thread t = new Thread(timer);
				t.start();

				this.recv.recvBuf = "";
				while (!TicTacToe.NotInGame) {
					if (this.recv.recvBuf == "")
						continue;
					if (Character.isDigit(this.recv.recvBuf.charAt(0)) &&
							this.recv.recvBuf.charAt(0) != '0')
						break;
					if (this.display.gameOverMsg != null)
						break;
				}
				if (TicTacToe.NotInGame) {
					// better way would be to put a lock in ActionListener for gameOverMsg.
					try {
						Thread.sleep(10);
					} catch (Exception e) {
					}
					try {
						gt.join();
					} catch (InterruptedException e) {
						System.err.println("Could not join giveup thread.");
						System.exit(1);
					}
					if (this.display.gameOverMsg != null && this.display.gameOverMsg.equals("Click to start.")) {
						if (this.c.isP1())
							this.display.gameOverMsg = "You have given up. Player 2 wins.";
						else
							this.display.gameOverMsg = "You have given up. Player 1 wins.";
						this.c.sendGiveup();
					}
/*					else if (this.display.gameOverMsg != null && this.display.gameOverMsg.contains("given")) {
						return;
					}*/
					else if (this.display.gameOverMsg != null && this.display.gameOverMsg.equals("disconnect")) {
/*						// server disconnect -> if recv ACK, then other player disconnected.
						this.c.sendWin(0);*/
						this.c.sendBye();
/*						String ack = "";
						try {
							ack = this.c.receiveFrom(1);
						} catch (Exception e) {
							this.display.gameOverMsg = "You lose. Possible connection loss.";
							e.printStackTrace();
						}
						if (ack.equals("ACK"))
							this.display.gameOverMsg = "Other player lost connection. You win.";*/
						this.display.gameOverMsg = "Connection loss.";
					}
					return;
				}

				input = this.recv.recvBuf.charAt(0) - '0';

				msg.gotMsg = true;
				try {
					t.join();
				} catch (InterruptedException e) {
					System.err.println("Could not join timer thread.");
					System.exit(1);
				}

				if (p1turn && !this.board.insert(this.p1.getSymbol(), input)) {
					System.err.println("Error with receivePosition with input: " + input);
					return;
//					System.exit(1);
				}
				if (!p1turn && !this.board.insert(this.p2.getSymbol(), input)) {
					System.err.println("Error with receivePosition with input: " + input);
					return;
//					System.exit(1);
				}
			}
			p1turn = !p1turn;
		}
	}
}
