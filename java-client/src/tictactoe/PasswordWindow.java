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
	private static String GUEST = "guest";
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
	
	public static boolean isValidCredential(String credential) {
		if (credential == "")
			return false;
		if (credential.length() >= VAR_SIZE)
			return false;
		
		for (int i = 0; i < credential.length(); i++) {
			if (!Character.isDigit(credential.charAt(i)) &&
					!Character.isLetter(credential.charAt(i))) {
				return false;
			}
		}
		return true;
	}

	private JPanel createButtonPanel() {
		JPanel p = new JPanel(new GridLayout(0, 1));
		JButton okButton = new JButton("OK");
		JButton guestButton = new JButton("GUEST");
		JButton quitButton = new JButton("QUIT");

		okButton.setActionCommand(PasswordWindow.OK);
		guestButton.setActionCommand(PasswordWindow.GUEST);
		quitButton.setActionCommand(PasswordWindow.QUIT);
		okButton.addActionListener(this);
		quitButton.addActionListener(this);
		guestButton.addActionListener(this);

		p.add(okButton);
		p.add(guestButton);
		p.add(quitButton);
		return p;
	}

	public void actionPerformed(ActionEvent e) {
		String cmd = e.getActionCommand();

		if (PasswordWindow.OK.equals(cmd)) {
			String currUser = _usernameField.getText();
			String currPass = new String(_passwordField.getPassword());
			if (!PasswordWindow.isValidCredential(currUser)) {
				JOptionPane.showMessageDialog(this, "Invalid username.");
				return;
			}
			if (!PasswordWindow.isValidCredential(currPass)) {
				JOptionPane.showMessageDialog(this, "Invalid password.");
				return;
			}
			Username = new String(_usernameField.getText());
			Password = new String(_passwordField.getPassword());
		}
		
		if (PasswordWindow.GUEST.equals(cmd)) {
			Username = new String("");
			Password = new String("");
		}

		if (PasswordWindow.QUIT.equals(cmd)) {
			System.exit(0);
		}
	}
}
