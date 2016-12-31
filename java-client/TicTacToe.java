import java.awt.Dimension;
import java.awt.Toolkit;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JLabel;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;

@SuppressWarnings("serial")
public final class TicTacToe extends JPanel {
	public final static int HEIGHT = 400;
	public final static int WIDTH = 400;

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
				if (TicTacToe.NotInGame)
					return;
				try {
					String test = this.c.receiveFromServer();
					this.recv.recvBuf = TicTacToe.stringToLength(test, "giveup".length());
				} catch (DisconnectException e) {
					if (TicTacToe.NotInGame)
						return;
					TicTacToe.NotInGame = true;
					if (this.c.isP1())
						this.display.gameOverMsg = "Player 2 has given up. You win.";
					else
						this.display.gameOverMsg = "Player 1 has given up. You win.";
					return;
				}
				if (this.recv.recvBuf != "" && this.recv.recvBuf.charAt(0) == 'w' && !TicTacToe.NotInGame) {
					TicTacToe.NotInGame = true;
					if (this.c.isP1()) {
						this.board.insert(Player.P2_SYMBOL, this.recv.recvBuf.charAt(1) - '0');
						this.display.doRepaint();
						this.display.gameOverMsg = "Player 2 wins.";
					}
					else {
						this.board.insert(Player.P1_SYMBOL, this.recv.recvBuf.charAt(1) - '0');
						this.display.doRepaint();
						this.display.gameOverMsg = "Player 1 wins.";
					}
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
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		window.setSize(TicTacToe.WIDTH, TicTacToe.HEIGHT);
		window.setLocation((screenSize.width - TicTacToe.WIDTH) / 2,
		(screenSize.height - TicTacToe.HEIGHT) / 2);
		window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		window.setResizable(false);
		window.setVisible(true);

		while (true) {
			TicTacToe game = new TicTacToe();
			window.setContentPane(game);

			game.start(username, password);
			System.out.println("Exited game.start()");
			game.getDisplay().doRepaint();

			while (TicTacToe.NotInGame)
				;
		}
	}

	public TicTacToe() {
		this.board = new Board();
		this.p1 = new Player(Player.P1_SYMBOL);
		this.p2 = new Player(Player.P2_SYMBOL);
		this.recv = new Recv();
		this.recv.recvBuf = "";
		this.c = new Client();

		setLayout(null);
		this.display = new Display(this.board);
		this.display.setPreferredSize(new Dimension(Display.WIDTH, Display.HEIGHT));
		this.display.setBounds(0, 70, Display.WIDTH, Display.HEIGHT);
		this.turnfield = new JLabel();
		this.turnfield.setBounds(0, 0, 100, 50);
		this.playerfield = new JLabel();
		this.playerfield.setBounds(150, 0, 200, 50);
		this.timerfield = new JLabel();
		this.timerfield.setBounds(360, 0, 30, 50);
		this.quitbutton = new JButton("Quit");
		add(this.display);
		add(this.turnfield);
		add(this.playerfield);
		add(this.timerfield);
		this.timerfield.setText("");
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
		this.quitbutton.setBounds(320, 100, 60, 60);
		add(this.quitbutton);
		this.winrecordfield = new JLabel();
		this.winrecordfield.setBounds(320, 200, 40, 40);
		add(this.winrecordfield);
		this.lossrecordfield = new JLabel();
		this.lossrecordfield.setBounds(320, 250, 40, 40);
		add(this.lossrecordfield);
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

		String[] initialSplit = this.c.Record.split(",");
		String winRecord = initialSplit[0].split("r")[1];
		String lossRecord = initialSplit[1];
		this.winrecordfield.setText("W: " + winRecord);
		this.lossrecordfield.setText("L: " + lossRecord);

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
					TicTacToe.NotInGame = true;
					this.display.doRepaint();
					c.sendWin(input);
					if (p1turn)
						this.display.gameOverMsg = "Player 1 wins.";
					else
						this.display.gameOverMsg = "Player 2 wins.";

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
					if (this.display.gameOverMsg.equals("Click to start.")) {
						if (this.c.isP1())
							this.display.gameOverMsg = "You have given up. Player 2 wins.";
						else
							this.display.gameOverMsg = "You have given up. Player 1 wins.";
					}
					this.c.sendGiveup();
					try {
						gt.join();
					} catch (InterruptedException e) {
						System.err.println("Could not join giveup thread.");
						System.exit(1);
					}
					return;
				}

				input = this.recv.recvBuf.charAt(0) - '0';
				this.recv.recvBuf = "";

				msg.gotMsg = true;
				try {
					t.join();
				} catch (InterruptedException e) {
					System.err.println("Could not join timer thread.");
					System.exit(1);
				}

				if (p1turn && !this.board.insert(this.p1.getSymbol(), input)) {
					System.err.println("Error with receivePosition with input: " + input);
					System.exit(1);
				}
				if (!p1turn && !this.board.insert(this.p2.getSymbol(), input)) {
					System.err.println("Error with receivePosition with input: " + input);
					System.exit(1);
				}
			}
			p1turn = !p1turn;
		}
	}
}
