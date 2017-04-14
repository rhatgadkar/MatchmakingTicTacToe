package tictactoe;

import javax.swing.JLabel;

public class MockTicTacToe implements ITicTacToe {
	
	private Board _board;
	private MockClient _c;
	private Game _game;
	private Display _display;
	
	public MockTicTacToe() {
		_board = new Board();
		_c = new MockClient();
		_game = new Game(this, _c, _board);
		_display = new Display(_game.getBoard());
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

	@Override
	public Game getGame() {
		return _game;
	}

	@Override
	public void repaintDisplay() {
	}

	@Override
	public void lockGameOverMsg() {
		_display.GameOverMsgLock.lock();
	}

	@Override
	public void unlockGameOverMsg() {
		_display.GameOverMsgLock.unlock();
	}

	@Override
	public void setGameOverMsg(String newMsg) {
		_display.GameOverMsg = newMsg;
	}

	@Override
	public String getGameOverMsg() {
		return _display.GameOverMsg;
	}

	@Override
	public int getInput(char symbol) {
		// TODO Auto-generated method stub
		return 0;
	}

}
