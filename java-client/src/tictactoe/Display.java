package tictactoe;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import javax.swing.JPanel;

@SuppressWarnings("serial")
public class Display extends JPanel implements MouseListener {
	
	public final static int WIDTH = 300;
	public final static int HEIGHT = 300;

	private class InputMsg {
		public volatile int input;
	}

	private Board _board;
	private GameOverMsg _gom;
	private boolean _allowInput;
	private final InputMsg _acceptedInput;
	private char _symbol;

	public Display(Board board, GameOverMsg gom) {
		addMouseListener(this);
		_board = board;
		_gom = gom;
		_allowInput = false;
		_acceptedInput = new InputMsg();
		_acceptedInput.input = -1;
		_symbol = 0;
		if (Game.NotInGame.get())
			_gom.setGameOverMsg(GameOverMsg.CLICK_TO_START);
		repaint();
	}

	private class InputThread implements Runnable {
		private InputMsg _msg;
		public InputThread(InputMsg msg) {
			_msg = msg;
		}
		@Override
		public void run() {
			while (_msg.input == -1 && !Game.NotInGame.get())
				;
		}
	}

	public int getInput(char symbol) {
		_acceptedInput.input = -1;
		Thread t = new Thread(new InputThread(_acceptedInput));
		t.start();

		_symbol = symbol;
		_allowInput = true;
		try {
			t.join();
		} catch (InterruptedException e) {
			System.err.println("Could not join input thread.");
			System.exit(1);
		}

		return _acceptedInput.input;
	}

	public void paintComponent(Graphics g) {
		synchronized (_gom) {
			if (_gom.getGameOverMsg() == null) {
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
						char symbol = _board.getSymbolAtCoord(tileRow, tileCol);
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
				g.drawString(_gom.getGameOverMsg(), 20, Display.HEIGHT / 2);
				if (!_gom.getGameOverMsg().equals(GameOverMsg.CLICK_TO_START))
					g.drawString(GameOverMsg.CLICK_TO_RESTART, 20,
							Display.HEIGHT - 100);
			}
		}
	}

	@Override
	public void mouseClicked(MouseEvent e) {}

	@Override
	public void mousePressed(MouseEvent e) {
		if (_allowInput) {
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
				boolean addTile = _board.insert(_symbol, input);
				if (addTile) {
					_acceptedInput.input = input;
					_allowInput = false;
				}
			}
		}
		else if (Game.NotInGame.get()) {
			synchronized (_gom) {
				if (_gom.getGameOverMsg() != null) {
					Game.NotInGame.set(false);
					_gom.setGameOverMsg(null);
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
