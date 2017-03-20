package tictactoe;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.net.SocketTimeoutException;
import javax.swing.JOptionPane;

public final class Client {
	public final static int MAXBUFLEN = 100;
	private final static String SERVERIP = "54.219.156.253";
//	private final static String SERVERIP = "192.168.218.140";
	private final static String SERVERPORT = "4950";

	private Socket _sock;
	private boolean _isP1;
	private String _username;
	private String _password;

	public boolean DoneInit;
	public String Record;

	public Client() {
		DoneInit = false;
	}

	public void init(String username, String password) {
		_username = username;
		_password = password;
		String buf = "";

		int retries;
		for (retries = 0; retries < 10; retries++) {
			try {
				_sock.close();
			} catch (Exception e) {
			}

			if (TicTacToe.NotInGame.get())
				break;

			// connect to parent server
			try {
				createSocketServer(SERVERPORT);
			} catch (Exception e) {
				System.err.println("Could not create socket to parent server. Exiting.");
				e.printStackTrace();
				continue;
			}
			try {
				int numPpl = getNumPpl();
				if (numPpl == -1) {
					System.out.println("Parent server is busy. Retrying.");
					Thread.sleep(15000);
					continue;
				}
				if (numPpl == 2) {
					System.out.println("Child servers are full. Retrying.");
					retries = 0;
					Thread.sleep(15000);
					continue;
				}
				buf = handleSynAck();
			} catch (Exception e) {
				continue;
			}

			// close connection to parent server
			try {
				_sock.close();
			} catch (IOException e) {
				System.err.println("Could not close connection to parent server. Exiting.");
				e.printStackTrace();
				continue;
			}

			// connect to child server
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				System.err.println("Could not sleep. Exiting.");
				e.printStackTrace();
				break;
			}
			try {
				createSocketServer(buf);
			} catch (Exception e) {
				System.err.println("Could not create socket to child server. Exiting.");
				e.printStackTrace();
				continue;
			}
			System.out.println("Connected to child server.");
			buf = "";
			try {
				buf = handleChildSynAck();
			} catch (Exception e) {
				// possible collision to a child server
				retries = 0;
				continue;
			}
			buf = TicTacToe.stringToLength(buf, "player-1".length());

			try {
				_sock.setSoTimeout(0);
			} catch (Exception e) {
				continue;
			}

			// get assigned player-1 or player-2
			if (buf.equals("player-1")) {
				_isP1 = true;
				boolean retryConn = false;
				do {
					buf = "";
					try {
						buf = receiveFrom(130);
					} catch (SocketTimeoutException e) {
						System.err.println("Couldn't find opponent.");
						retryConn = true;
						break;
					} catch (Exception e) {
						System.err.println("Child server exited.");
						e.printStackTrace();
						retryConn = true;
						break;
					}
				} while (buf.charAt(0) != 'r' && buf.charAt(0) != ',' &&
						buf.charAt(0) != 'b');
				if (retryConn)
					continue;
				if (buf.charAt(0) == 'b') {
					retries = 0;
					System.out.println("Didn't find opponent. Searching again.");
					continue;
				}
				System.out.println("Current record: " + buf);
				this.Record = new String(buf);
				break;
			}
			else if (buf.charAt(0) == 'r' || buf.charAt(0) == ',') {
				if (buf.charAt(0) == 'r') {
					System.out.println("Current record: " + buf);
					this.Record = new String(buf);
				}
				_isP1 = false;
				break;
			}
			else if (buf.equals("invalidl")) {
				JOptionPane.showMessageDialog(null, "Invalid login credentials. Exiting.");
				System.exit(0);
			}
			else {
				JOptionPane.showMessageDialog(null, "User is currently in game. Exiting.");
				System.exit(0);
			}
		}
		if (retries == 10) {
			JOptionPane.showMessageDialog(null, "Connection to server failed.");
			System.exit(1);
		}
		DoneInit = true;
	}

	public void close() {
		if (_sock != null) {
			try {
				_sock.close();
			} catch (IOException e) {
			}
		}
	}

	public void sendPosition(int pos) {
		sendToServer(Integer.toString(pos));
	}

	public boolean isP1() {
		return _isP1;
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

	public void sendTie(int pos) {
		String a = "t" + Integer.toString(pos);
		sendToServer(a);
	}

	private String handleSynAck() throws Exception {
		String ack = "";
		try {
			ack = receiveFrom(15);
			System.out.println("Receieved ACK from server.");
		} catch (Exception e) {
			System.err.println("Possible server disconnect.");
			e.printStackTrace();
			throw new Exception();
		}
		return ack;
	}

	private String handleChildSynAck() throws Exception {
		String login = _username + "," + _password;
		sendToServer(login);
		String ack = "";
		try {
			ack = receiveFrom(15);
			System.out.println("Receieved ACK from server.");
		} catch (Exception e) {
			System.err.println("Possible server disconnect.");
			e.printStackTrace();
			throw new Exception();
		}
		return ack;
	}

	private void createSocketServer(String port) throws Exception {
		port = TicTacToe.stringToLength(port, SERVERPORT.length());
		try {
			_sock = new Socket(SERVERIP, Integer.parseInt(port));
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

	private int getNumPpl() throws Exception {
		try {
			String numPpl = receiveFrom(15);
			if (numPpl.charAt(0) == 'b')
				return 2;
			System.out.println("Number of people online: " + numPpl);
			return 1;
		} catch (Exception e) {
			System.err.println("Possible server disconnect.");
			e.printStackTrace();
			throw new Exception();
		}
	}

	private void sendToServer(String msg) {
		byte message[] = new byte[Client.MAXBUFLEN];
		int i;
		for (i = 0; i < msg.length(); i++)
			message[i] = (byte)(msg.charAt(i));
		for (; i < Client.MAXBUFLEN; i++)
			message[i] = 0;
		try {
			OutputStream os = _sock.getOutputStream();
			os.write(message, 0, Client.MAXBUFLEN);
			os.flush();
		} catch (IOException e) {
			System.err.println("Error send message.");
			e.printStackTrace();
			return;
		}
	}

	public String receiveFrom(int sec) throws SocketTimeoutException,
			Exception {
		byte message[] = new byte[Client.MAXBUFLEN];
		try {
			InputStream in = _sock.getInputStream();
			_sock.setSoTimeout(sec * 1000);
			int status;
			for (int numbytes = 0; numbytes < Client.MAXBUFLEN; numbytes += status) {
				status = in.read(message, numbytes, Client.MAXBUFLEN - numbytes);
				if (status == -1)
					throw new Exception();
			}
		} catch (SocketTimeoutException e) {
			throw new SocketTimeoutException();
		} catch (Exception e) {
			throw new Exception();
		}
		int nullLoc;
		for (nullLoc = 0; nullLoc < Client.MAXBUFLEN; nullLoc++) {
			if (message[nullLoc] == 0)
				break;
		}
		return new String(message, 0, nullLoc);
	}

	public String receiveFromServer() throws Exception {
		byte message[] = new byte[Client.MAXBUFLEN];
		try {
			InputStream in = _sock.getInputStream();
			int status;
			for (int numbytes = 0; numbytes < Client.MAXBUFLEN; numbytes += status) {
				status = in.read(message, numbytes, Client.MAXBUFLEN - numbytes);
				if (status == -1)
					throw new Exception();
			}
		} catch (IOException e) {
			System.err.println("Error receive message.");
			e.printStackTrace();
			throw new Exception();
		}
		int nullLoc;
		for (nullLoc = 0; nullLoc < Client.MAXBUFLEN; nullLoc++) {
			if (message[nullLoc] == 0)
				break;
		}
		return new String(message, 0, nullLoc);
	}
}
