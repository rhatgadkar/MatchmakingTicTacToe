import java.io.Console;

public final class TicTacToe {
	private Board board;
	private Player p1;
	private Player p2;
	private String[] recvBuf;
	
	private class CheckGiveupThread implements Runnable {
		private String[] recvBuf;
        private Client c;
        private Board board;
		public CheckGiveupThread(String[] recvBuf, Client c) {
			this.recvBuf = recvBuf;
			this.c = c;
		}
		@Override
		public void run() {
			do {
				try {
					String test = this.c.receiveFromServer();
					StringBuilder sb = new StringBuilder(test);
					sb.setLength("giveup".length());
					this.recvBuf[0] = sb.toString();
				} catch (DisconnectException e) {
					try {
						Thread.sleep(100);
					} catch (InterruptedException ie) {}
					System.out.println("Client is exiting. Closing server.");
					System.exit(1);
				}
			} while (!this.recvBuf[0].equals("giveup"));
			if (this.c.isP1())
				System.out.println("Player 2 has given up. Player 1 wins.");
			else
				System.out.println("Player 1 has given up. Player 2 wins.");
			System.exit(0);
		}
	}
	
	public static void main(String[] args) {
		TicTacToe game = new TicTacToe();
		game.start();
	}
	
	public TicTacToe() {
		this.board = new Board();
		this.p1 = new Player('x');
		this.p2 = new Player('o');
		this.recvBuf = new String[1];
		this.recvBuf[0] = "";
	}
	
	public void start() {
		Client c = new Client();
		
		Runnable giveupThread = new CheckGiveupThread(this.recvBuf, c);
		Thread gt = new Thread(giveupThread);
		gt.start();
		
		boolean p1turn = true;
		while (true) {
			// draw board
			if (p1turn && c.isP1())
				System.out.println("Your turn.");
			else if (p1turn && !c.isP1())
				System.out.println("Player 1 turn.");
			else
				System.out.println("Your turn.");
			
			this.board.draw();
			
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
				System.out.println(msg.gotMsg);
				
				c.sendPosition(input);
			}
			// wait for other player to make move
			else {
				final TimerThread.Msg msg = new TimerThread.Msg();
				msg.gotMsg = false;
				Runnable timer = new TimerThread(msg, 90);
				Thread t = new Thread(timer);
				t.start();
				
				while (true) {
					try {
						// TODO: The below is a "way around" some
						// synchronization issue with this.recvBuf. It should
						// use proper synchronization instead of sleeping.
						Thread.sleep(100);
					} catch (InterruptedException e) {}
					if (this.recvBuf[0] == "")
						continue;
					if (Character.isDigit(this.recvBuf[0].charAt(0)) &&
							this.recvBuf[0].charAt(0) != '0')
						break;
				}
				input = this.recvBuf[0].charAt(0) - '0';
				this.recvBuf[0] = "";
				
				System.out.println("input: " + input);
				
				msg.gotMsg = true;
				try {
					t.join();
				} catch (InterruptedException e) {
					System.err.println("Could not join timer thread.");
					System.exit(1);
				}
				
				if (p1turn && !this.board.insert(this.p1.getSymbol(), input)) {
					System.out.println("Error with receivePosition with input: " + input);
					return;
				}
				if (!p1turn && !this.board.insert(this.p2.getSymbol(), input)) {
					System.out.println("Error with receivePosition with input: " + input);
					return;
				}
			}
			
			// check if win
			if (this.board.isWin(input)) {
				if (p1turn)
					System.out.println("Player 1 wins.");
				else
					System.out.println("Player 2 wins.");
				this.board.draw();
				return;
			}
			
			p1turn = !p1turn;
		}
	}
}
