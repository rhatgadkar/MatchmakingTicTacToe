import java.util.Date;
import javax.swing.JLabel;

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
		while (!this.msg.gotMsg && (elapsedTime < this.seconds * 1000) && !TicTacToe.NotInGame) {
			elapsedTime = (new Date().getTime()) - startTime;
			if (this.display == null) {
				Long currCountdown = (long)this.seconds - (elapsedTime / 1000);
				if (currCountdown != countdown) {
					countdown = currCountdown;
					this.timerCountdown.setText(currCountdown.toString());
				}
			}
		}
		if (!this.msg.gotMsg && !TicTacToe.NotInGame) {
			TicTacToe.NotInGame = true;
			System.out.println(this.errorMsg);
			if (this.display != null)
				this.display.gameOverMsg = "Error with opponent. Match canceled.";
		}
	}
}
