import java.awt.Color;
import java.awt.Graphics;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import javax.swing.JPanel;

@SuppressWarnings("serial")
public class Display extends JPanel implements MouseListener {
	
	public final static int WIDTH = 300;
	public final static int HEIGHT = 300;

	public String gameOverMsg;

	private class InputMsg {
		public volatile int input;
	}

	private Board board;
	private boolean allowInput;
	private final InputMsg acceptedInput;
	private char symbol;

	public Display(Board board) {
		addMouseListener(this);
		this.board = board;
		this.allowInput = false;
		this.acceptedInput = new InputMsg();
		this.acceptedInput.input = -1;
		this.symbol = 0;
		if (TicTacToe.NotInGame) {
			synchronized (this) {
				this.gameOverMsg = "Click to start.";
			}
		}
		repaint();
	}

	private class InputThread implements Runnable {
		private InputMsg msg;
		public InputThread(InputMsg msg) {
			this.msg = msg;
		}
		@Override
		public void run() {
			while (this.msg.input == -1 && !TicTacToe.NotInGame)
				;
		}
	}

	public int getInput(char symbol) {
		this.acceptedInput.input = -1;
		Thread t = new Thread(new InputThread(this.acceptedInput));
		t.start();

		this.symbol = symbol;
		this.allowInput = true;
		try {
			t.join();
		} catch (InterruptedException e) {
			System.err.println("Could not join input thread.");
			System.exit(1);
		}

		return this.acceptedInput.input;
	}

	public void doRepaint() {
		repaint();
	}

	public void paintComponent(Graphics g) {
		if (this.gameOverMsg == null) {
			g.setColor(Color.lightGray);
			g.fillRect(0, 0, Display.WIDTH, Display.HEIGHT);
			g.setColor(Color.BLACK);
			g.drawLine(100, 0, 100, Display.HEIGHT);
			g.drawLine(200, 0, 200, Display.HEIGHT);
			g.drawLine(0, 100, Display.WIDTH, 100);
			g.drawLine(0, 200, Display.WIDTH, 200);

			for (int tileRow = 0; tileRow < Board.ROWS; tileRow++) {
				for (int tileCol = 0; tileCol < Board.COLS; tileCol++) {
					int x = tileCol * (Display.WIDTH / Board.COLS);
					int y = tileRow * (Display.HEIGHT / Board.ROWS);
					char symbol = this.board.getSymbolAtCoord(tileRow, tileCol);
					g.setColor(Color.BLACK);
					if (symbol == 'x') {
						g.drawLine(x + 20, y + 20, x + 20 + 50, y + 20 + 50);
						g.drawLine(x + 20, y + 20 + 50, x + 20 + 50, y + 20);
					}
					else if (symbol == 'o')
						g.drawOval(x + 20, y + 20, 50, 50);
				}
			}
		}
		else {
			g.setColor(Color.lightGray);
			g.fillRect(0, 0, Display.WIDTH, Display.HEIGHT);
			g.setColor(Color.BLACK);
			g.drawString(this.gameOverMsg, 20, Display.HEIGHT / 2);
			if (!this.gameOverMsg.equals("Click to start."))
				g.drawString("Click to restart.", 20, Display.HEIGHT - 100);
		}
	}

	@Override
	public void mouseClicked(MouseEvent e) {}

	@Override
	public void mousePressed(MouseEvent e) {
		if (this.allowInput) {
			int row = e.getY() / (Display.HEIGHT / Board.ROWS);
			int col = e.getX() / (Display.WIDTH / Board.COLS);
			if (row == Board.ROWS)
				row = Board.ROWS - 1;
			if (col == Board.COLS)
				col = Board.COLS - 1;

			if (col >= 0 && col < Board.COLS && row >= 0 && row < Board.ROWS) {
				int input;
				if (row == 0)
					input = col + 1;
				else if (row == 1)
					input = row * 4 + col;
				else
					input = row * 3 + col + 1;
				boolean addTile = this.board.insert(this.symbol, input);
				if (addTile) {
					this.acceptedInput.input = input;
					this.allowInput = false;
				}
			}
		}
		else if (TicTacToe.NotInGame) {
			if (this.gameOverMsg != null) {
				TicTacToe.NotInGame = false;
				synchronized (this) {
					this.gameOverMsg = null;
				}
			}
		}
	}

	@Override
	public void mouseReleased(MouseEvent e) {}

	@Override
	public void mouseEntered(MouseEvent e) {}

	@Override
	public void mouseExited(MouseEvent e) {}
}
