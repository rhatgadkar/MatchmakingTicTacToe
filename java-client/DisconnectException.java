package tictactoe;

@SuppressWarnings("serial")
public class DisconnectException extends Exception {
	public DisconnectException() {}
	public DisconnectException(String message) {
		super(message);
	}
	public DisconnectException(Throwable cause) {
		super(cause);
	}
	public DisconnectException(String message, Throwable cause) {
		super(message, cause);
	}
}
