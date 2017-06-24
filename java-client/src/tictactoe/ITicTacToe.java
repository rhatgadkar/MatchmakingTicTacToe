package tictactoe;

import javax.swing.JLabel;

public interface ITicTacToe {
	public final static String CLICK_TO_START = "Click to start.";
	public final static String CLICK_TO_RESTART = " Press 'Start' to restart.";
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
	
	public void setGameOverMsg(String newMsg);
	public String getGameOverMsg();
	
	public void setTurnfieldText(String text);
	public void setPlayerfieldText(String text);
	public void setTimerfieldText(String text);
	public void setWinfieldText(String text);
	public void setLossfieldText(String text);
	public void setOpponentText(String text);
	public void setNumPplText(String text);
	public JLabel getTimerfield();
	public void repaintDisplay();
	public int getInput(char symbol);
	public void runGame();
	public void handlePlayerLogin(String[] args);
	public void showGameOverDialog(String message);
	public void setQuitbuttonVisible(boolean enable);
	public void setStartbuttonVisible(boolean enable);
}
