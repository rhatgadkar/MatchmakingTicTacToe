import java.awt.Dimension;
import java.awt.Toolkit;
import javax.swing.JFrame;
import javax.swing.JPanel;
import java.io.Console;

@SuppressWarnings("serial")
public final class TicTacToe extends JPanel {
	public final static int HEIGHT = 350;
	public final static int WIDTH = 350;

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
				try {
					String test = this.c.receiveFromServer();
					this.recv.recvBuf = TicTacToe.stringToLength(test, "giveup".length());
				} catch (DisconnectException e) {
					System.out.println("Client is exiting. Closing server.");
					System.exit(1);
				}
				if (this.recv.recvBuf != "" && this.recv.recvBuf.charAt(0) == 'w') {
					if (this.c.isP1()) {
						System.out.println("Player 2 wins.");
						this.board.insert('o', this.recv.recvBuf.charAt(1) - '0');
					}
					else {
						System.out.println("Player 1 wins.");
						this.board.insert('x', this.recv.recvBuf.charAt(1) - '0');
					}
					this.display.doRepaint();
					System.out.println("Client is exiting. Closing server.");
					System.exit(0);
				}
			} while (!this.recv.recvBuf.equals("giveup"));
			if (this.c.isP1())
				System.out.println("Player 2 has given up. Player 1 wins.");
			else
				System.out.println("Player 1 has given up. Player 2 wins.");
			System.exit(0);
		}
	}

	public static void main(String[] args) {
		JFrame window = new JFrame("Matchmaking TicTacToe");
		TicTacToe game = new TicTacToe();
		window.setContentPane(game);

		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		window.setSize(TicTacToe.WIDTH, TicTacToe.HEIGHT);
		window.setLocation((screenSize.width - TicTacToe.WIDTH) / 2,
		(screenSize.height - TicTacToe.HEIGHT) / 2);
		window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		window.setResizable(false);
		window.setVisible(true);

		game.start();
	}

	public TicTacToe() {
		this.board = new Board();
		this.p1 = new Player('x');
		this.p2 = new Player('o');
		this.recv = new Recv();
		this.recv.recvBuf = "";

		setLayout(null);
		this.display = new Display(this.board);
		this.display.setPreferredSize(new Dimension(Display.WIDTH, Display.HEIGHT));
		this.display.setBounds(0, 0, Display.WIDTH, Display.HEIGHT);
		add(this.display);
	}

	public void start() {
		Client c = new Client();

		Runnable giveupThread = new CheckGiveupThread(this.recv, c, this.board, this.display);
		Thread gt = new Thread(giveupThread);
		gt.start();

		boolean p1turn = true;
		while (true) {
			// draw board
			if (p1turn && c.isP1())
				System.out.println("Your turn.");
			else if (p1turn && !c.isP1())
				System.out.println("Player 1 turn.");
			else if (!p1turn && c.isP1())
				System.out.println("Player 2 turn.");
			else
				System.out.println("Your turn.");

			this.display.doRepaint();

			int input;
			// insert at position
			if ((p1turn && c.isP1()) || (!p1turn && !c.isP1())) {
				final TimerThread.Msg msg = new TimerThread.Msg();
				msg.gotMsg = false;

				Runnable timer = new TimerThread(msg, 60);
				Thread t = new Thread(timer);
				t.start();

				while (true) {
					Console console = System.console();
					if (console == null)
						System.out.println("console is null");
					String inputStr = "";
					while (true) {
						try {
							inputStr = console.readLine("Enter position (1-9): ");
							input = Integer.parseInt(inputStr);
							break;
						} catch (Exception e) {}
					}

					if (p1turn && !this.board.insert(this.p1.getSymbol(), input))
						continue;
					if (!p1turn && !this.board.insert(this.p2.getSymbol(), input))
						continue;
					break;
				}

				msg.gotMsg = true;
				try {
					t.join();
				} catch (InterruptedException e) {
					System.err.println("Could not join timer thread.");
					System.exit(1);
				}

				c.sendPosition(input);
				// check if win
				if (this.board.isWin(input)) {
					if (p1turn)
						System.out.println("Player 1 wins.");
					else
						System.out.println("Player 2 wins.");
					this.display.doRepaint();
					try {
						Thread.sleep(100);
					} catch (Exception e) {}
					c.sendWin(input);
					System.exit(0);
				}
			}
			// wait for other player to make move
			else {
				final TimerThread.Msg msg = new TimerThread.Msg();
				msg.gotMsg = false;
				Runnable timer = new TimerThread(msg, 90);
				Thread t = new Thread(timer);
				t.start();

				while (true) {
					if (this.recv.recvBuf == "")
						continue;
					if (Character.isDigit(this.recv.recvBuf.charAt(0)) &&
							this.recv.recvBuf.charAt(0) != '0')
						break;
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
					System.out.println("Error with receivePosition with input: " + input);
					System.exit(1);
				}
				if (!p1turn && !this.board.insert(this.p2.getSymbol(), input)) {
					System.out.println("Error with receivePosition with input: " + input);
					System.exit(1);
				}
			}
			p1turn = !p1turn;
		}
	}
}
