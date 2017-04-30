package tictactoe;

public class GameOverMsg {
	private String _gameOverMsg;
	
	public final static String CLICK_TO_START = "Click to start.";
	public final static String CLICK_TO_RESTART = "Click to restart.";
	public final static String YOU_WIN = "You win.";
	public final static String TIE_GAME = "Tie game.";
	public final static String CONNECTION_LOSS = "Connection loss.";
	public final static String NO_PLAY_MOVE =
			"You have not played a move. You lose.";
	public final static String GIVEN_UP = "You have given up. You lose.";
	public final static String DISCONNECT = "disconnect";
	public final static String P1_GIVEUP_WIN =
			"Player 1 has given up. You win.";
	public final static String P2_GIVEUP_WIN = 
			"Player 2 has given up. You win.";
	public final static String P2_WIN_LOSE = "Player 2 wins. You lose.";
	public final static String P1_WIN_LOSE = "Player 1 wins. You lose.";
	
	public synchronized void setGameOverMsg(String newMsg) {
		_gameOverMsg = newMsg;
	}
	
	public synchronized String getGameOverMsg() {
		return _gameOverMsg;
	}
}
