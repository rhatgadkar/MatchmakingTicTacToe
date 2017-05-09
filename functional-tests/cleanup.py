import os
from screenutils import list_screens
import re

screen_name = r's[1-9][0-9]*'
screen_name_prog = re.compile(screen_name)
for s in list_screens():
    if screen_name_prog.match(s.name):
        s.kill()
os.system('rm *.log')
