package tictactoe;

import javax.swing.JLabel;

public interface ITicTacToe {
	
	public void setTurnfieldText(String text);
	public void setPlayerfieldText(String text);
	public void setTimerfieldText(String text);
	public void setWinfieldText(String text);
	public void setLossfieldText(String text);
	public void setOpponentText(String text);
	public JLabel getTimerfield();
	public void repaintDisplay();
	public int getInput(char symbol);
	public void runGame();
	public void handlePlayerLogin(String[] args);
	public void showGameOverDialog(String message);
}
