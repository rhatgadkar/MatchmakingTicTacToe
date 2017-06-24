package tictactoe;

public class MockTicTacToe implements ITicTacToe {
	
	private String _gameOverMsg;
	private Board _board;
	private MockClient _c;
	private Game _game;
	int _currMove;
	String _turnfieldText;
	String _playerfieldText;
	String _timerfieldText;
	String _lossfieldText;
	String _winfieldText;
	
	public MockTicTacToe(boolean isP1, int currMove, String opponentMove,
			Board b) {
		/**
		 * - currMove contains integer from 1-9 representing positions in
		 *   Board.  -1 means send a "giveup".
		 * - opponentMove contains String: "giveup", "w#", "t#", "1"-"9"
		 *   representing the received value of an opponent's move.
		 */
		
		_board = b;
		_c = new MockClient(isP1, opponentMove);
		_game = new Game(this, _c, _board);
		_currMove = currMove;
		// after "Click to start", NotInGame==false and GameOverMsg==null
		Game.NotInGame.set(false);
		setGameOverMsg(null);
	}

	@Override
	public void setTurnfieldText(String text) {
		_turnfieldText = text;
	}
	
	public String getTurnfieldText() {
		return _turnfieldText;
	}

	@Override
	public void setPlayerfieldText(String text) {
		_playerfieldText = text;
	}
	
	public String getPlayerfieldText() {
		return _playerfieldText;
	}

	@Override
	public void setTimerfieldText(String text) {
		_timerfieldText = text;
	}
	
	public String getTimerfieldText() {
		return _timerfieldText;
	}

	@Override
	public void setWinfieldText(String text) {
		_winfieldText = text;
	}
	
	public String getWinfieldText() {
		return _winfieldText;
	}

	@Override
	public void setLossfieldText(String text) {
		_lossfieldText = text;
	}
	
	public String getLossfieldText() {
		return _lossfieldText;
	}

	@Override
	public void repaintDisplay() {
	}

	@Override
	public int getInput(char symbol) {
		/**
		 * If _currMove is -1, this means a "giveup" should be sent immediately.
		 * If _currMove is 0, stall until timer expiry for "giveup".
		 */
		if (_currMove == 0) {
			while (!Game.NotInGame.get())
				;
			return -1;
		}
		if (_currMove == -1) {
			// "Click to start." is set when player forcefully gives up
			synchronized (this) {
				Game.NotInGame.set(true);
				setGameOverMsg(ITicTacToe.CLICK_TO_START);
			}
			return -1;
		}
		_board.insert(symbol, _currMove);
		return _currMove;
	}
	
	public String getFinalMessage() {
		return _c.getFinalMsg();
	}

	@Override
	public void runGame() {		
	}

	@Override
	public void handlePlayerLogin(String[] args) {
		// TODO Auto-generated method stub
		
	}

	public Game getGame() {
		return _game;
	}

	@Override
	public void showGameOverDialog(String message) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void setOpponentText(String text) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void setNumPplText(String text) {
		// TODO Auto-generated method stub
		
	}
	
	public synchronized void setGameOverMsg(String newMsg) {
		_gameOverMsg = newMsg;
	}
	
	public synchronized String getGameOverMsg() {
		return _gameOverMsg;
	}

	@Override
	public void setQuitbuttonVisible(boolean enable) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void setStartbuttonVisible(boolean enable) {
		// TODO Auto-generated method stub
		
	}

}
