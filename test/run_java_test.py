from os import system
import sys


CMD = 'java -cp %s tictactoe.TicTacToe %s'
CMD = CMD % (sys.argv[1], sys.argv[2:])
while True:
    a = system(CMD)
    if a != 0:
        break
print 'Error happened. Couldn\'t start java client. Exiting.'
