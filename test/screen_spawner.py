from screenutils import Screen
from os import system
import sys

# arg0: screen_spawner.py
# arg1: number of screen sessions
# arg2: 'y' for login. o.w. no login

if len(sys.argv) < 3 or len(sys.argv) > 3:
    print 'There must be 2 arguments for screen sessions and login.'
    sys.exit()

cmd = ''

while True:
    user_input = raw_input("Press '1' for cpp or '2' for java: ")
    if user_input == '1':
        cmd = 'python run_cpp_test.py'
        break
    if user_input == '2':
        cmd = 'python run_java_test.py'
        break

for i in range(0, int(sys.argv[1])):
    scr_str = str(i + 1)
    s = Screen('s' + scr_str, True)
    s.enable_logs('s' + scr_str + '.log')
    if sys.argv[2] == 'y':
        s.send_commands(cmd + ' s' + scr_str)
    else:
        s.send_commands(cmd)
