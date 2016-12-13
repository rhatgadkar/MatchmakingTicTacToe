import java.util.Date;

public class TimerThread implements Runnable {
	public static class Msg {
		public volatile boolean gotMsg;
	}

	private Msg msg;
	private int seconds;
	private String errorMsg;
	private Display display;

	public TimerThread(Msg msg, int seconds, String errorMsg, Display display) {
		this.msg = msg;
		this.seconds = seconds;
		this.errorMsg = errorMsg;
		this.display = display;
	}
	@Override
	public void run() {
		long startTime = System.currentTimeMillis();
		long elapsedTime = 0L;
		while (!this.msg.gotMsg && (elapsedTime < this.seconds * 1000) && !TicTacToe.NotInGame) {
			elapsedTime = (new Date().getTime()) - startTime;
		}
		if (!this.msg.gotMsg && !TicTacToe.NotInGame) {
			TicTacToe.NotInGame = true;
			System.out.println(this.errorMsg);
			if (this.display != null)
				this.display.gameOverMsg = "Error with opponent. Match canceled.";
		}
	}
}
