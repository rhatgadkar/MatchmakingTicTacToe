import java.util.Date;
import javax.swing.JLabel;
import java.util.concurrent.atomic.AtomicBoolean;

public class TimerThread implements Runnable {
	public static class Msg {
		public volatile boolean gotMsg;
	}

	private Msg msg;
	private int seconds;
	private String errorMsg;
	private Display display;
	private JLabel timerCountdown;

	public TimerThread(Msg msg, int seconds, String errorMsg, Display display, JLabel timerCountdown) {
		this.msg = msg;
		this.seconds = seconds;
		this.errorMsg = errorMsg;
		this.display = display;
		this.timerCountdown = timerCountdown;
	}
	@Override
	public void run() {
		long startTime = System.currentTimeMillis();
		long elapsedTime = 0L;
		long countdown = 0L;
		while (!this.msg.gotMsg && (elapsedTime < this.seconds * 1000) && !TicTacToe.NotInGame.get()) {
			elapsedTime = (new Date().getTime()) - startTime;
			if (this.display == null) {
				Long currCountdown = (long)this.seconds - (elapsedTime / 1000);
				if (currCountdown != countdown) {
					countdown = currCountdown;
					this.timerCountdown.setText(currCountdown.toString());
				}
			}
		}
		if (!this.msg.gotMsg && !TicTacToe.NotInGame.get()) {
			TicTacToe.NotInGame.set(true);
			if (this.display != null) {
				synchronized (this.display) {
					this.display.gameOverMsg = "disconnect";
				}
			}
			System.out.println(this.errorMsg);
		}
	}
}
