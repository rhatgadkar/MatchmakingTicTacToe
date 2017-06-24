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

@SuppressWarnings("serial")
public final class TicTacToe extends JPanel implements ITicTacToe {
	
	private String _gameOverMsg;
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
	private JLabel _numpplfield;
	private String _username;
	private String _password;
	private JButton _startbutton;
	
	public void repaintDisplay() {
		_display.repaint();
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

			while (Game.NotInGame.get())
				;
			_c.close();
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
		JPanel g2 = new JPanel(new GridLayout(2, 1));
		g2.add(_opponentnamefield);
		g2.add(_numpplfield);
		grid.add(g2);
		return grid;
	}

	private JPanel createRightGrid() {
		JPanel p = new JPanel(new GridLayout(4, 1));
		p.add(_quitbutton);
		p.add(_startbutton);
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
		_display = new Display(_game.getBoard(), this);
		_display.setPreferredSize(new Dimension(Display.WIDTH,
				Display.HEIGHT));
		_turnfield = new JLabel();
		_playerfield = new JLabel();
		_timerfield = new JLabel();
		_quitbutton = new JButton("Quit");
		_startbutton = new JButton("Start");
		_timerfield.setText("");
		_winrecordfield = new JLabel();
		_lossrecordfield = new JLabel();
		_opponentnamefield = new JLabel();
		_numpplfield = new JLabel();

		_quitbutton.addActionListener(new ActionListener() {
			private Client _c;
			public void actionPerformed(ActionEvent e) {
				if (!_c.getDoneInit()) {
					setPlayerfieldText("Quitting...");
					synchronized (this) {
						_c.close();
					}
				}
				setPlayerfieldText("");
				Game.NotInGame.set(true);
				setGameOverMsg(ITicTacToe.CLICK_TO_START);
				_quitbutton.setVisible(false);
				if (!_c.getDoneInit())
					System.exit(0);
			}
			private ActionListener init(Client c) {
				_c = c;
				return this;
			}
		}.init(_c));
		
		_startbutton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				synchronized (this) {
					if (getGameOverMsg() != null) {
						Game.NotInGame.set(false);
						setGameOverMsg(null);
					}
				}
			}
		});
		
		setQuitbuttonVisible(false);

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
	
	public void setQuitbuttonVisible(boolean enable) {
		_quitbutton.setVisible(enable);
	}
	
	public void setStartbuttonVisible(boolean enable) {
		_startbutton.setVisible(enable);
	}
	
	public void setQuitbuttonText(String text) {
		_quitbutton.setText(text);
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
		_numpplfield.setText(message);
	}

	@Override
	public void setOpponentText(String text) {
		_opponentnamefield.setText(text);
	}

	@Override
	public void setNumPplText(String text) {
		_numpplfield.setText(text);
	}
	
	public synchronized void setGameOverMsg(String newMsg) {
		_gameOverMsg = newMsg;
	}
	
	public synchronized String getGameOverMsg() {
		return _gameOverMsg;
	}
}
