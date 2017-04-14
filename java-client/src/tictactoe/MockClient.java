package tictactoe;

import java.net.SocketTimeoutException;

public class MockClient implements IClient {

	@Override
	public String receiveFrom(int i) throws SocketTimeoutException, Exception {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean isP1() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void sendGiveup() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void sendBye() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void sendWin(int input) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void sendTie(int input) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void sendPosition(int input) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void init(String username, String password) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public String getRecord() {
		// TODO Auto-generated method stub
		return null;
	}

}
