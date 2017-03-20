package tictactoe;

import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.JTextField;
import javax.swing.JOptionPane;

@SuppressWarnings("serial")
public class PasswordWindow extends JPanel implements ActionListener {
	private static String OK = "ok";
	private static String QUIT = "quit";
	private static int VAR_SIZE = 21;

	private JTextField _usernameField;
	private JPasswordField _passwordField;

	public volatile String Username;
	public volatile String Password;

	public PasswordWindow() {
		_usernameField = new JTextField(PasswordWindow.VAR_SIZE - 1);
		_passwordField = new JPasswordField(PasswordWindow.VAR_SIZE - 1);

		JLabel usernameLabel = new JLabel("Enter username (max " +
				Integer.toString(VAR_SIZE - 1) +
				" letters, only letters and digits allowed):");
		usernameLabel.setLabelFor(_usernameField);

		JLabel passwordLabel = new JLabel("Enter password (max " +
				Integer.toString(VAR_SIZE - 1) +
				" letters, only letters and digits allowed):");
		passwordLabel.setLabelFor(_passwordField);

		JPanel buttonPane = createButtonPanel();

		JPanel textPane = new JPanel(new GridLayout(0, 1));
		textPane.add(usernameLabel);
		textPane.add(_usernameField);
		textPane.add(passwordLabel);
		textPane.add(_passwordField);

		add(textPane);
		add(buttonPane);
	}

	private JPanel createButtonPanel() {
		JPanel p = new JPanel(new GridLayout(0, 1));
		JButton okButton = new JButton("OK");
		JButton quitButton = new JButton("QUIT");

		okButton.setActionCommand(PasswordWindow.OK);
		quitButton.setActionCommand(PasswordWindow.QUIT);
		okButton.addActionListener(this);
		quitButton.addActionListener(this);

		p.add(okButton);
		p.add(quitButton);
		return p;
	}

	public void actionPerformed(ActionEvent e) {
		String cmd = e.getActionCommand();

		if (PasswordWindow.OK.equals(cmd)) {
			String currUser = _usernameField.getText();
			String currPass = new String(_passwordField.getPassword());
			if (currUser.length() >= VAR_SIZE || currPass.length() >= VAR_SIZE) {
				JOptionPane.showMessageDialog(null,
						"Username and Password must be at most " +
						Integer.toString(VAR_SIZE - 1) + " characters long.");
				return;
			}
			if (currUser.length() != 0 && currPass.length() != 0) {
				for (int i = 0; i < currUser.length(); i++) {
					if (!Character.isDigit(currUser.charAt(i)) &&
							!Character.isLetter(currUser.charAt(i))) {
						JOptionPane.showMessageDialog(null,
								"Username and Password must only contain letters and digits.");
						return;
					}
				}
				for (int i = 0; i < currPass.length(); i++) {
					if (!Character.isDigit(currPass.charAt(i)) &&
							!Character.isLetter(currPass.charAt(i))) {
						JOptionPane.showMessageDialog(null,
								"Username and Password must only contain letters and digits.");
						return;
					}
				}
			}
			Username = new String(_usernameField.getText());
			Password = new String(_passwordField.getPassword());
		}

		if (PasswordWindow.QUIT.equals(cmd)) {
			System.exit(0);
		}
	}
}
