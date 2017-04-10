package tictactoe;

import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.FlowLayout;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JLabel;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import java.util.concurrent.atomic.AtomicBoolean;

@SuppressWarnings("serial")
public final class TicTacToe extends JPanel {

	public static AtomicBoolean NotInGame = new AtomicBoolean(true);

	public static String stringToLength(String input, int length) {
		StringBuilder sb = new StringBuilder(input);
		sb.setLength(length);
		return sb.toString();
	}

	private Board _board;
	private Client _c;
	private Game _game;
	private Display _display;
	private JLabel _turnfield;
	private JLabel _playerfield;
	private JLabel _timerfield;
	private JButton _quitbutton;
	private JLabel _winrecordfield;
	private JLabel _lossrecordfield;

	public Display getDisplay() {
		return _display;
	}

	public static void main(String[] args) {
		// login
		JFrame login = new JFrame("Login");
		login.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		PasswordWindow loginPrompt = new PasswordWindow();
		login.setContentPane(loginPrompt);
		login.pack();
		login.setVisible(true);
		while (loginPrompt.Username == null && loginPrompt.Password == null)
			;
		String username = new String(loginPrompt.Username);
		String password = new String(loginPrompt.Password);
		login.setVisible(false);
		login.dispose();

		JFrame window = new JFrame("Matchmaking TicTacToe");
		window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		while (true) {
			Board b = new Board();
			Client c = new Client();
			Display d = new Display(b);
			d.setPreferredSize(new Dimension(Display.WIDTH,
					Display.HEIGHT));
			TicTacToe ttt = new TicTacToe(b, c, d);
			window.setContentPane(ttt);
			window.pack();
			window.setVisible(true);

			ttt.getGame().start(username, password);
			System.out.println("Exited game.start()");
			ttt.getDisplay().doRepaint();

			while (TicTacToe.NotInGame.get())
				;
		}
	}

	private JPanel createTopPanel() {
		JPanel p = new JPanel(new FlowLayout(FlowLayout.CENTER, 50, 0));
		p.add(_turnfield);
		p.add(_playerfield);
		p.add(_timerfield);
		return p;
	}

	private JPanel createRightGrid() {
		JPanel p = new JPanel(new GridLayout(3, 1));
		p.add(_quitbutton);
		p.add(_winrecordfield);
		p.add(_lossrecordfield);
		return p;
	}

	private JPanel createBotPanel() {
		JPanel p = new JPanel(new FlowLayout());
		p.add(_display);
		p.add(createRightGrid());
		return p;
	}

	public TicTacToe(Board b, Client c, Display d) {
		_board = b;
		_c = c;
		_game = new Game(this, _c, _board);
		_display = d;
		_turnfield = new JLabel();
		_playerfield = new JLabel();
		_timerfield = new JLabel();
		_quitbutton = new JButton("Quit");
		_timerfield.setText("");
		_winrecordfield = new JLabel();
		_lossrecordfield = new JLabel();


		_quitbutton.addActionListener(new ActionListener() {
			private Display _display;
			private Client _c;
			public void actionPerformed(ActionEvent e) {
				if (TicTacToe.NotInGame.get() || !_c.DoneInit)
					System.exit(0);
				else {
					_display.GameOverMsgLock.lock();
					try {
						TicTacToe.NotInGame.set(true);
						_display.GameOverMsg = "Click to start.";
					} finally {
						_display.GameOverMsgLock.unlock();
					}
				}
			}
			private ActionListener init(Display display, Client c) {
				_display = display;
				_c = c;
				return this;
			}
		}.init(_display, _c));

		setLayout(new GridLayout(2, 1));
		add(createTopPanel());
		add(createBotPanel());
	}

	public void setTurnfieldText(String text) {
		_turnfield.setText(text);
	}

	public void setPlayerfieldText(String text) {
		_playerfield.setText(text);
	}

	public void setTimerfieldText(String text) {
		_timerfield.setText(text);
	}

	public void setWinfieldText(String text) {
		_winrecordfield.setText(text);
	}

	public void setLossfieldText(String text) {
		_lossrecordfield.setText(text);
	}

	public JLabel getTimerfield() {
		return _timerfield;
	}

	public Game getGame() {
		return _game;
	}
}
