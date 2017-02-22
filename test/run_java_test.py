from os import system

cmd = 'java -cp test-java-client/ TicTacToe 1 2 3 4 5 6 7 8 9'

while True:
    a = system(cmd)
    if a != 0:
        break
print 'Error happened. Couldn\'t start test-java-client. Exiting.'
