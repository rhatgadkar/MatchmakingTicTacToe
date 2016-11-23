import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.net.UnknownHostException;

public final class Client {
	public final static int MAXBUFLEN = 1000;
	private final static String SERVERIP = "54.183.217.40";
//	private final static String SERVERIP = "127.0.0.1";
	private final static String SERVERPORT = "4950";
	
	private Socket sock;
	private boolean isP1;
	
	public Client() {
		String buf = "";
		
		// connect to parent server
		try {
			createSocketServer(SERVERPORT);
		} catch (Exception e) {
			System.err.println("Could not create socket to parent server. Exiting.");
			System.exit(1);
		}
		getNumPpl();
		buf = handleSynAck();
		
		// close connection to parent server
		try {
			this.sock.close();
		} catch (IOException e) {
			System.err.println("Could not close connection to parent server. Exiting.");
			System.exit(1);
		}
		
		// connect to child server
		try {
			Thread.sleep(1000);
		} catch (InterruptedException e) {
			System.err.println("Could not sleep. Exiting.");
			System.exit(1);
		}
		try {
			createSocketServer(buf);
		} catch (Exception e) {
			System.err.println("Could not create socket to child server. Exiting.");
			System.exit(1);
		}
		System.out.println("Connected to child server.");
		buf = "";
		buf = handleSynAck();
        buf = TicTacToe.stringToLength(buf, "player-1".length());

		// get assigned player-1 or player-2
		if (buf.equals("player-1")) {
			System.out.println("You are player 1.");
			this.isP1 = true;
			System.out.println("Waiting for player 2 to connect...");
			do {
				buf = "";
				try {
					buf = receiveFromServer();
                    buf = TicTacToe.stringToLength(buf, "player-2".length());
				} catch (DisconnectException e) {
					System.err.println("Child server exited.");
					System.exit(1);
				}
			} while (!buf.equals("player-2"));
			System.out.println("Player 2 has connected. Starting game.");
		}
		else if (buf.equals("player-2")) {
			System.out.println("You are player 2.");
			this.isP1 = false;
		}
		else {
			System.out.println("Try connecting again: " + buf);
			System.exit(0);
		}
	}
	
	public void sendPosition(int pos) {
		sendToServer(Integer.toString(pos));
	}
	
	public boolean isP1() {
		return this.isP1;
	}
	
	public void sendGiveup() {
		sendToServer("giveup");
	}
	
	public void sendBye() {
		sendToServer("bye");
	}

    public void sendWin(int pos) {
        String a = "w" + Integer.toString(pos);
        sendToServer(a);
    }
	
	private String handleSynAck() {
		final TimerThread.Msg msg = new TimerThread.Msg();
		msg.gotMsg = false;
		Runnable timer = new TimerThread(msg, 15);
		Thread t = new Thread(timer);
		t.start();
		String ack = "";
		try {
			ack = receiveFromServer();
			msg.gotMsg = true;
			try {
				t.join();
			} catch (InterruptedException e) {
				System.err.println("Could not join timer thread.");
				System.exit(1);
			}
			System.out.println("Receieved ACK from server.");
		} catch (DisconnectException e) {
			System.out.println("Server disconnected. Exiting.");
			System.exit(0);
		}
		return ack;
	}
	
	private void createSocketServer(String port) throws Exception {
        port = TicTacToe.stringToLength(port, SERVERPORT.length());
		try {
			this.sock = new Socket(SERVERIP, Integer.parseInt(port));
		} catch (UnknownHostException e) {
			System.err.println("Can't create socket. Unknown host.");
			throw new Exception();
		} catch (IOException e) {
			System.err.println("Can't create socket. I/O error.");
			throw new Exception();
		} catch (NumberFormatException e) {
			System.err.println("Could not parse port string to int.");
			throw new Exception();
		}
	}
	
	private void getNumPpl() {
		final TimerThread.Msg msg = new TimerThread.Msg();
		msg.gotMsg = false;
		Runnable timer = new TimerThread(msg, 15);
		Thread t = new Thread(timer);
		t.start();
		try {
			String numPpl = receiveFromServer();
			msg.gotMsg = true;
			try {
				t.join();
			} catch (InterruptedException e) {
				System.err.println("Could not join timer thread.");
				System.exit(1);
			}
			System.out.println("Number of people online: " + numPpl);
		} catch (DisconnectException e) {
			System.out.println("Server disconnected. Exiting.");
			System.exit(0);
		}
	}
	
	private void sendToServer(String msg) {
		try {
			OutputStream os = this.sock.getOutputStream();
	        OutputStreamWriter osw = new OutputStreamWriter(os);
	        BufferedWriter bw = new BufferedWriter(osw);
	        bw.write(msg);
	        bw.flush();
		} catch (IOException e) {
			System.err.println("Error send message.");
			System.exit(1);
		}
	}
	
	public String receiveFromServer() throws DisconnectException {
		char message[] = new char[Client.MAXBUFLEN];
		try {
			InputStream is = this.sock.getInputStream();
			InputStreamReader isr = new InputStreamReader(is);
			BufferedReader br = new BufferedReader(isr);
			if (br.read(message, 0, Client.MAXBUFLEN) == -1)
				throw new DisconnectException();
		} catch (IOException e) {
			System.err.println("Error receive message.");
			System.exit(1);
		}
		return new String(message);
	}
}
