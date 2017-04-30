package tictactoe;

import javax.swing.JLabel;

public class FvtTicTacToe implements ITicTacToe {
	
	private String _gameOverMsg;
	private Board _board;
	private Client _c;
	private Game _game;
	private Display _display;
	private String _username;
	private String _password;
	private String[] _moves;
	
	public FvtTicTacToe() {
		_board = new Board();
		_c = new Client();
		_game = new Game(this, _c, _board);
		_display = new Display(_game.getBoard(), this);
		// after "Click to start", NotInGame==false and GameOverMsg==null
		Game.NotInGame.set(false);
		setGameOverMsg(null);
	}

	@Override
	public void setTurnfieldText(String text) {
	}

	@Override
	public void setPlayerfieldText(String text) {
	}

	@Override
	public void setTimerfieldText(String text) {
	}

	@Override
	public void setWinfieldText(String text) {
	}

	@Override
	public void setLossfieldText(String text) {
	}

	@Override
	public JLabel getTimerfield() {
		return null;
	}
	
	public void repaintDisplay() {
		_display.repaint();
	}

	@Override
	public int getInput(char symbol) {
		int input = -1;
		int movesIter = 0;

		while (!Game.NotInGame.get()) {
			// add 250 millisecond delay before getting move
			try {
				Thread.sleep(250);
			} catch (Exception e) {
				input = -1;
				continue;
			}
			
			if (movesIter >= _moves.length) {
				input = -1;
				continue;
			}
			else
				input = _moves[movesIter].charAt(0) - '0';
			
			if (!_board.insert(symbol, input)) {
				movesIter++;
				input = -1;
				continue;
			}
			
			break;
		}
		
		return input;
	}

	@Override
	public void handlePlayerLogin(String[] args) {
		_username = new String(args[0]);
		_password = new String(args[0]);
		_moves = new String[args.length - 1];
		for (int k = 1; k < args.length; k++)
			_moves[k - 1] = args[k];
	}

	@Override
	public void runGame() {
		_game.start(_username, _password);
		System.out.println("Exited game.start()");
		System.exit(0);
	}

	@Override
	public void showGameOverDialog(String message) {
	}

	@Override
	public void setOpponentText(String text) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public synchronized void setGameOverMsg(String newMsg) {
		_gameOverMsg = newMsg;
		System.out.println(newMsg);
	}

	@Override
	public synchronized String getGameOverMsg() {
		return _gameOverMsg;
	}

}
