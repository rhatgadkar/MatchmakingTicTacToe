import java.awt.Dimension;
import java.awt.Toolkit;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JLabel;

@SuppressWarnings("serial")
public final class TicTacToe extends JPanel {
	public final static int HEIGHT = 400;
	public final static int WIDTH = 400;

	public static volatile boolean win = true;

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
				if (TicTacToe.win)
					return;
				try {
					String test = this.c.receiveFromServer();
					this.recv.recvBuf = TicTacToe.stringToLength(test, "giveup".length());
				} catch (DisconnectException e) {
					if (TicTacToe.win)
						return;
					System.out.println("Disconnect");
					System.exit(1);
				}
				if (this.recv.recvBuf != "" && this.recv.recvBuf.charAt(0) == 'w' && !TicTacToe.win) {
					TicTacToe.win = true;
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
			if (!TicTacToe.win) {
				TicTacToe.win = true;
				if (this.c.isP1())
					this.display.gameOverMsg = "Player 2 has given up. Player 1 wins.";
				else
					this.display.gameOverMsg = "Player 1 has given up. Player 2 wins.";
				return;
			}
		}
	}

	public static void main(String[] args) {
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

			game.start();
			System.out.println("Exited game.start()");
			game.getDisplay().doRepaint();

			while (TicTacToe.win)
				;
		}
	}

	public TicTacToe() {
		this.board = new Board();
		this.p1 = new Player(Player.P1_SYMBOL);
		this.p2 = new Player(Player.P2_SYMBOL);
		this.recv = new Recv();
		this.recv.recvBuf = "";

		setLayout(null);
		this.display = new Display(this.board);
		this.display.setPreferredSize(new Dimension(Display.WIDTH, Display.HEIGHT));
		this.display.setBounds(0, 70, Display.WIDTH, Display.HEIGHT);
		this.turnfield = new JLabel();
		this.turnfield.setBounds(0, 0, 100, 50);
		this.playerfield = new JLabel();
		this.playerfield.setBounds(150, 0, 200, 50);
		add(this.display);
		add(this.turnfield);
		add(this.playerfield);
	}

	public void start() {
		if (TicTacToe.win) {
			this.playerfield.setText("MatchMaking TicTacToe");
			while (TicTacToe.win)
				;
			this.display.doRepaint();
		}

		this.playerfield.setText("Searching for opponent...");

		Client c = new Client();

		if (c.isP1())
			this.playerfield.setText("You are player 1 (" + Player.P1_SYMBOL + ").");
		else
			this.playerfield.setText("You are player 2 (" + Player.P2_SYMBOL + ").");

		Runnable giveupThread = new CheckGiveupThread(this.recv, c, this.board, this.display);
		Thread gt = new Thread(giveupThread);
		gt.start();

		boolean p1turn = true;
		while (!TicTacToe.win) {
			// draw board
			if (p1turn && c.isP1())
				this.turnfield.setText("Your turn.");
			else if (p1turn && !c.isP1())
				this.turnfield.setText("Player 1 turn.");
			else if (!p1turn && c.isP1())
				this.turnfield.setText("Player 2 turn.");
			else
				this.turnfield.setText("Your turn.");

			this.display.doRepaint();

			int input;
			// insert at position
			if ((p1turn && c.isP1()) || (!p1turn && !c.isP1())) {
				final TimerThread.Msg msg = new TimerThread.Msg();
				msg.gotMsg = false;
				String errorMsg =
					"You have not played a move in 60 seconds. You have given up.";
				Runnable timer = new TimerThread(msg, 60, errorMsg);
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

				// check if win
				if (this.board.isWin(input) && !TicTacToe.win) {
					TicTacToe.win = true;
					this.display.doRepaint();
					c.sendWin(input);
					if (p1turn)
						this.display.gameOverMsg = "Player 1 wins.";
					else
						this.display.gameOverMsg = "Player 2 wins.";

					c.sendBye();
					return;
				}
				else {
					if (input == -1) {
						if (p1turn)
							this.display.gameOverMsg = "You have not played a move. Player 2 wins.";
						else
							this.display.gameOverMsg = "You have not played a move. Player 1 wins.";
						c.sendGiveup();
						return;
					}
					else
						c.sendPosition(input);
				}
			}
			// wait for other player to make move
			else {
				final TimerThread.Msg msg = new TimerThread.Msg();
				msg.gotMsg = false;
				String errorMsg =
					"A move has not been received in 90 seconds. Closing connection.";
				Runnable timer = new TimerThread(msg, 90, errorMsg);
				Thread t = new Thread(timer);
				t.start();

				while (!TicTacToe.win) {
					if (this.recv.recvBuf == "")
						continue;
					if (Character.isDigit(this.recv.recvBuf.charAt(0)) &&
							this.recv.recvBuf.charAt(0) != '0')
						break;
				}
				if (TicTacToe.win) {
					try {
						gt.join();
					} catch (InterruptedException e) {
						System.err.println("Could not join giveup thread.");
						System.exit(1);
					}

					c.sendBye();
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
