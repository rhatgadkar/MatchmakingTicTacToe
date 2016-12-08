import java.util.Date;

public class TimerThread implements Runnable {
	public static class Msg {
		public volatile boolean gotMsg;
	}

	private Msg msg;
	private int seconds;
	private String errorMsg;

	public TimerThread(Msg msg, int seconds, String errorMsg) {
		this.msg = msg;
		this.seconds = seconds;
		this.errorMsg = errorMsg;
	}
	@Override
	public void run() {
		long startTime = System.currentTimeMillis();
		long elapsedTime = 0L;
		while (!this.msg.gotMsg && (elapsedTime < this.seconds * 1000)) {
			elapsedTime = (new Date().getTime()) - startTime;
		}
		if (!this.msg.gotMsg) {
			System.out.println(this.errorMsg);
//			System.exit(0);
		}
	}
}
