package tictactoe;

import java.util.Date;
import javax.swing.JLabel;

public class TimerThread implements Runnable {
	public static class Msg {
		public volatile boolean gotMsg;
	}

	private Msg _msg;
	private int _seconds;
	private JLabel _timerCountdown;

	public TimerThread(Msg msg, int seconds, JLabel timerCountdown) {
		_msg = msg;
		_seconds = seconds;
		_timerCountdown = timerCountdown;
	}
	@Override
	public void run() {
		long startTime = System.currentTimeMillis();
		long elapsedTime = 0L;
		long countdown = 0L;
		while (!_msg.gotMsg && (elapsedTime < _seconds * 1000) &&
				!Game.NotInGame.get()) {
			elapsedTime = (new Date().getTime()) - startTime;
			if (_timerCountdown != null) {
				Long currCountdown = (long)_seconds - (elapsedTime / 1000);
				if (currCountdown != countdown) {
					countdown = currCountdown;
					_timerCountdown.setText(currCountdown.toString());
				}
			}
		}
		if (!_msg.gotMsg && !Game.NotInGame.get())
			Game.NotInGame.set(true);
	}
}
