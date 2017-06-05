package tictactoe;

import java.util.Date;

public class TimerThread implements Runnable {
	public static class Msg {
		public volatile boolean gotMsg;
	}

	private Msg _msg;
	private int _seconds;
	private ITicTacToe _ttt;

	public TimerThread(Msg msg, int seconds, ITicTacToe ttt) {
		_msg = msg;
		_seconds = seconds;
		_ttt = ttt;
	}
	@Override
	public void run() {
		long startTime = System.currentTimeMillis();
		long elapsedTime = 0L;
		long countdown = 0L;
		while (!_msg.gotMsg && (elapsedTime < _seconds * 1000) &&
				!Game.NotInGame.get()) {
			elapsedTime = (new Date().getTime()) - startTime;
			if (_ttt != null) {
				Long currCountdown = (long)_seconds - (elapsedTime / 1000);
				if (currCountdown != countdown) {
					countdown = currCountdown;
					_ttt.setTimerfieldText(currCountdown.toString());
				}
			}
		}
		if (!_msg.gotMsg && !Game.NotInGame.get())
			Game.NotInGame.set(true);
	}
}
