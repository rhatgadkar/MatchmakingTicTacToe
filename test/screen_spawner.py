from screenutils import Screen
from os import system
import sys

cmd = 'python run_cpp_test.py'

if len(sys.argv) <= 1 or len(sys.argv) > 2:
    print 'There must be 1 argument for number of screen sessions.'
    sys.exit()

#screens_list = []
for i in range(0, int(sys.argv[1])):
    scr_str = str(i + 1)
    s = Screen('s-' + scr_str, True)
    s.enable_logs('s-' + scr_str + '.log')
#    screens_list.append(s)
    s.send_commands(cmd)
