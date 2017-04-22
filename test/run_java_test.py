from os import system
import sys


CMD = 'java -cp %s tictactoe.TicTacToe %s'
working_dir = sys.argv[1]
tictactoe_args = ' '.join(sys.argv[2:])
CMD = CMD % (working_dir, tictactoe_args)
print CMD
while True:
    a = system(CMD)
    if a != 0:
        break
print 'Error happened. Couldn\'t start java client. Exiting.'
