public final class Player {
	public static final char P1_SYMBOL = 'x';
	public static final char P2_SYMBOL = 'o';

	private char _symbol;
	
	public Player(char symbol) {
		_symbol = symbol;
	}
	
	public char getSymbol() {
		return _symbol;
	}
}
