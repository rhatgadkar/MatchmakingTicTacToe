package tictactoe;

import java.net.SocketTimeoutException;

public class MockClient implements IClient {
	
	private boolean _isP1;
	private String _opponentMove;
	private String _finalMsg;
	
	public MockClient(boolean isP1, String opponentMove) {
		_isP1 = isP1;
		_opponentMove = opponentMove;
		_finalMsg = "";
	}

	@Override
	public String receiveFrom(int i) throws SocketTimeoutException, Exception {
		return _opponentMove;
	}

	@Override
	public boolean isP1() {
		return _isP1;
	}

	@Override
	public void sendGiveup() {
		_finalMsg = "giveup";
	}

	@Override
	public void sendBye() {
		_finalMsg = "bye";		
	}

	@Override
	public void sendWin(int pos) {
		_finalMsg = "w" + Integer.toString(pos);		
	}

	@Override
	public void sendTie(int pos) {
		_finalMsg = "t" + Integer.toString(pos);		
	}

	@Override
	public void sendPosition(int pos) {
		_finalMsg = Integer.toString(pos);
	}

	@Override
	public void init(String username, String password, ITicTacToe ttt) {		
	}

	@Override
	public String getRecord() {
		return "r,,";
	}

	@Override
	public String getNumPpl() {
		return "";
	}

	public String getFinalMsg() {
		return _finalMsg;
	}

	@Override
	public boolean getDoneInit() {
		return true;
	}
}
