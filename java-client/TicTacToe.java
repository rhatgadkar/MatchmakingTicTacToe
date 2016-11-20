import java.io.Console;

public final class TicTacToe {
	private Board board;
	private Player p1;
	private Player p2;
    private Recv recv;

    private class Recv {
        public volatile String recvBuf;
    }
	
	private class CheckGiveupThread implements Runnable {
        private final Recv recv;
        private Client c;
        private Board board;
		public CheckGiveupThread(Recv recv, Client c, Board board) {
            this.recv = recv;
			this.c = c;
            this.board = board;
		}
		@Override
		public void run() {
			do {
				try {
					String test = this.c.receiveFromServer();
					StringBuilder sb = new StringBuilder(test);
					sb.setLength("giveup".length());
                    this.recv.recvBuf = sb.toString();
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
                    this.board.draw();
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
		TicTacToe game = new TicTacToe();
		game.start();
	}
	
	public TicTacToe() {
		this.board = new Board();
		this.p1 = new Player('x');
		this.p2 = new Player('o');
        this.recv = new Recv();
        this.recv.recvBuf = "";
	}
	
	public void start() {
		Client c = new Client();

        Runnable giveupThread = new CheckGiveupThread(this.recv, c, this.board);
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
                // check if win
                if (this.board.isWin(input)) {
                    if (p1turn)
                        System.out.println("Player 1 wins.");
                    else
                        System.out.println("Player 2 wins.");
                    this.board.draw();
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
