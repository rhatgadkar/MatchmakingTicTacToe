import java.awt.Color;
import java.awt.Graphics;
import javax.swing.JPanel;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.locks.ReentrantLock;

@SuppressWarnings("serial")
public class Display extends JPanel {
	
	public final static int WIDTH = 300;
	public final static int HEIGHT = 300;

	public String gameOverMsg;
	public ReentrantLock gameOverMsgLock = new ReentrantLock();

	private Board board;
	private boolean allowInput;
	private char symbol;

	public Display(Board board) {
		this.board = board;
		this.allowInput = false;
		this.symbol = 0;
		if (TicTacToe.NotInGame.get()) {
			this.gameOverMsgLock.lock();
			try {
				this.gameOverMsg = null;
			} finally {
				this.gameOverMsgLock.unlock();
			}
		}
		repaint();
	}

	public void doRepaint() {
		repaint();
	}

	public void paintComponent(Graphics g) {
		this.gameOverMsgLock.lock();
		try {
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
		} finally {
			this.gameOverMsgLock.unlock();
		}
	}

}
