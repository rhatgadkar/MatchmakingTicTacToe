package tictactoe;

import java.net.SocketTimeoutException;

public interface IClient {

	String receiveFrom(int i) throws SocketTimeoutException, Exception;

	boolean isP1();

	void sendGiveup();

	void sendBye();

	void sendWin(int input);

	void sendTie(int input);

	void sendPosition(int input);

	void init(String username, String password);

	String getRecord();

}
