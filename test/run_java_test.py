from os import system
import sys

cmd = 'java -cp test-java-client/ TicTacToe 1 2 3 4 5 6 7 8 9'

if len(sys.argv) < 1 or len(sys.argv) > 2:
	print 'There can only be 1 or 2 arguments.'
	sys.exit()
if len(sys.argv) == 2:
	cmd = cmd + ' ' + sys.argv[1]

while True:
    a = system(cmd)
    if a != 0:
        break
print 'Error happened. Couldn\'t start test-java-client. Exiting.'
