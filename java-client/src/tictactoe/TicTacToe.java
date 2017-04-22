package tictactoe;

import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.FlowLayout;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JOptionPane;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import java.util.concurrent.atomic.AtomicBoolean;

@SuppressWarnings("serial")
public final class TicTacToe extends JPanel implements ITicTacToe {

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
	private JLabel _opponentnamefield;
	private String _username;
	private String _password;
	
	public void repaintDisplay() {
		_display.repaint();
	}
	
	public void lockGameOverMsg() {
		_display.GameOverMsgLock.lock();
	}
	
	public void unlockGameOverMsg() {
		_display.GameOverMsgLock.unlock();
	}
	
	public void setGameOverMsg(String newMsg) {
		_display.GameOverMsg = newMsg;
	}
	
	public String getGameOverMsg() {
		return _display.GameOverMsg;
	}
	
	public int getInput(char symbol) {
		return _display.getInput(symbol);
	}
	
	public void handlePlayerLogin(String[] args) {
		JFrame login = new JFrame("Login");
		login.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		PasswordWindow loginPrompt = new PasswordWindow();
		login.setContentPane(loginPrompt);
		login.pack();
		login.setVisible(true);
		while (loginPrompt.Username == null && loginPrompt.Password == null)
			;
		_username = new String(loginPrompt.Username);
		_password = new String(loginPrompt.Password);
		login.setVisible(false);
		login.dispose();
	}
	
	public void runGame() {
		JFrame window = new JFrame("Matchmaking TicTacToe");
		window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		while (true) {
			initializeInterface();
			window.setContentPane(this);
			window.pack();
			window.setVisible(true);

			_game.start(_username, _password);
			System.out.println("Exited game.start()");
			repaintDisplay();

			while (TicTacToe.NotInGame.get())
				;
			removeAll();
		}
	}

	public static void main(String[] args) {		
		ITicTacToe ttt;
		if (args.length == 0)
			ttt = new TicTacToe();
		else
			ttt = new FvtTicTacToe();
		
		ttt.handlePlayerLogin(args);
		ttt.runGame();
	}

	private JPanel createTopPanel() {
		JPanel grid = new JPanel(new GridLayout(2, 1));
		JPanel p = new JPanel(new FlowLayout(FlowLayout.CENTER, 50, 0));
		p.add(_turnfield);
		p.add(_playerfield);
		p.add(_timerfield);
		grid.add(p);
		grid.add(_opponentnamefield);
		return grid;
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

	private void initializeInterface() {
		_board = new Board();
		_c = new Client();
		_game = new Game(this, _c, _board);
		_display = new Display(_game.getBoard());
		_display.setPreferredSize(new Dimension(Display.WIDTH,
				Display.HEIGHT));
		_turnfield = new JLabel();
		_playerfield = new JLabel();
		_timerfield = new JLabel();
		_quitbutton = new JButton("Quit");
		_timerfield.setText("");
		_winrecordfield = new JLabel();
		_lossrecordfield = new JLabel();
		_opponentnamefield = new JLabel();

		_quitbutton.addActionListener(new ActionListener() {
			private Display _display;
			private Client _c;
			public void actionPerformed(ActionEvent e) {
				if (TicTacToe.NotInGame.get() || !_c.getDoneInit())
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

	@Override
	public void showGameOverDialog(String message) {
		JOptionPane.showMessageDialog(this, message);
	}

	@Override
	public void setOpponentText(String text) {
		_opponentnamefield.setText(text);
	}
}
