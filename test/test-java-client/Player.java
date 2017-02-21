public final class Player {
	public static final char P1_SYMBOL = 'x';
	public static final char P2_SYMBOL = 'o';

	private char symbol;
	
	public Player(char symbol) {
		this.symbol = symbol;
	}
	
	public char getSymbol() {
		return this.symbol;
	}
}
