package tictactoe;

import java.net.SocketTimeoutException;

public interface IClient {

	String receiveFrom(int i) throws SocketTimeoutException, Exception;

	boolean isP1();

	void sendGiveup();

	void sendBye();

	void sendWin(int pos);

	void sendTie(int pos);

	void sendPosition(int pos);

	void init(String username, String password, ITicTacToe ttt);

	String getRecord();

	String getNumPpl();

	boolean getDoneInit();
}
