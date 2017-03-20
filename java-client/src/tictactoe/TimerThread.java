package tictactoe;

import java.util.Date;
import javax.swing.JLabel;

public class TimerThread implements Runnable {
	public static class Msg {
		public volatile boolean gotMsg;
	}

	private Msg _msg;
	private int _seconds;
	private String _errorMsg;
	private Display _display;
	private JLabel _timerCountdown;

	public TimerThread(Msg msg, int seconds, String errorMsg, Display display,
			JLabel timerCountdown) {
		_msg = msg;
		_seconds = seconds;
		_errorMsg = errorMsg;
		_display = display;
		_timerCountdown = timerCountdown;
	}
	@Override
	public void run() {
		long startTime = System.currentTimeMillis();
		long elapsedTime = 0L;
		long countdown = 0L;
		while (!_msg.gotMsg && (elapsedTime < _seconds * 1000) &&
				!TicTacToe.NotInGame.get()) {
			elapsedTime = (new Date().getTime()) - startTime;
			if (_display == null) {
				Long currCountdown = (long)_seconds - (elapsedTime / 1000);
				if (currCountdown != countdown) {
					countdown = currCountdown;
					_timerCountdown.setText(currCountdown.toString());
				}
			}
		}
		if (!_msg.gotMsg && !TicTacToe.NotInGame.get()) {
			TicTacToe.NotInGame.set(true);
			if (_display != null) {
				_display.GameOverMsgLock.lock();
				try {
					_display.GameOverMsg = "disconnect";
				} finally {
					_display.GameOverMsgLock.unlock();
				}
			}
			System.out.println(_errorMsg);
		}
	}
}
