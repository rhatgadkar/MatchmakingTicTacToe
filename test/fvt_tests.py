import argparse
from screenutils import Screen, list_screens
import table_parser as tp
from commands import getoutput
from time import sleep
import os
import re


WORKING_DIR = ''
DEPLOY_SCREENS_CMD = 'python run_java_test.py'
PSQL_PREFIX_CMD = "psql -d mydb -c "
PSQL_SELECT_CMD = "'" + PSQL_PREFIX_CMD + '"SELECT * FROM tttrecords;"' + "'"
PSQL_DELETE_RECORDS_CMD = "'" + PSQL_PREFIX_CMD + \
        '"DELETE FROM tttrecords;"' + "'"
PSQL_DELETE_LOGIN_CMD = "'" + PSQL_PREFIX_CMD + '"DELETE FROM tttlogin;"' + "'"
PS_CMD = "ps ww"
SERVER_ADDR = "ubuntu@54.219.156.253"


def get_raw_psql_table_output(identity_file):
    """
    Gets the raw psql output of the tttrecords table.
    """
    ssh_cmd = 'ssh -i ' + args.identity_file + ' ' + SERVER_ADDR
    get_tttrecords_cmd = ssh_cmd + ' ' + PSQL_SELECT_CMD
    raw_data = getoutput(get_tttrecords_cmd)
    return raw_data


def deploy_screens(num_screens, move_str):
    """
    Deploy the screen session's for each client using the run_java_test.py
    script with provided arguments of the working directory and move set.
    """
    for i in range(0, num_screens):
        scr_name = 's' + str(i + 1)
        fvt_args = WORKING_DIR + ' ' + scr_name + ' ' + move_str
        s = Screen(scr_name, True)
        s.enable_logs(scr_name + '.log')
        s.send_commands(DEPLOY_SCREENS_CMD + ' ' + fvt_args)


def kill_screens():
    """
    Destroy all running screen sessions. This also terminates the corresponding
    clients.
    """
    for s in list_screens():
        s.kill()


def get_file_wins_losses(screen_log):
    """
    Get wins and losses from screen session's log file.
    """
    f = None
    try:
        f = open(screen_log, 'r')
    except:
        raise('%s cannot be opened for reading.' % (screen_log))
    win_regex = r'.*You win.*'
    loss_regex = r'.*You lose.*'
    win_prog = re.compile(win_regex)
    loss_prog = re.compile(loss_regex)
    file_wins = 0
    file_losses = 0
    for line in f:
        if win_prog.match(line):
            file_wins += 1
        elif loss_prog.match(line):
            file_losses += 1
    f.close()
    return (file_wins, file_losses)


def verify_wins_losses(num_screens, psql_table_data):
    """
    Verify the wins and loss count from the screen session logs and psql table
    output.

    Example psql_table_data:
    [['s1', '3', '3'], ['s2', '3', '3'], ['s3', '3', '3']]
    """
    for data_list in psql_table_data:
        username = data_list[0]
        table_wins = int(data_list[1])
        table_losses = int(data_list[2])
        screen_log = os.getcwd() + '/' + username + '.log'
        (file_wins, file_losses) = get_file_wins_losses(screen_log)
        if file_wins == table_wins - 1 or file_wins == table_wins + 1 or \
                file_wins == table_wins:
            if file_losses == table_losses - 1 or \
                    file_losses == table_losses + 1 or \
                    file_losses == table_losses:
                        continue
        print 'File and table wins and losses are not within +/-1.'
        print 'file wins: ' + str(file_wins)
        print 'file losses: ' + str(file_losses)
        print 'table wins: ' + str(table_wins)
        print 'table losses: ' + str(table_losses)
        return False
    return True


def cleanup(identity_file):
    """
    Delete screen session logs and clear both psql records and login tables.
    """
    # delete screen logs
    screen_log_regex = r'^s[1-9][0-9]*\.log'
    screen_log_prog = re.compile(screen_log_regex)
    dir_items = os.listdir(os.getcwd())
    for item in dir_items:
        if screen_log_prog.match(item):
            os.system('rm ' + os.getcwd() + '/' + item)
    # delete psql table entries
    ssh_cmd = 'ssh -i ' + identity_file + ' ' + SERVER_ADDR + ' ' + \
            PSQL_DELETE_RECORDS_CMD
    os.system(ssh_cmd)
    ssh_cmd = 'ssh -i ' + identity_file + ' ' + SERVER_ADDR + ' ' + \
            PSQL_DELETE_LOGIN_CMD
    os.system(ssh_cmd)


# tests:
def default_test(identity_file):
    """
    5 clients are running for 10 minutes with move set of '1 2 3 4 5 6 7 8 9'.
    """
    move_str = '1 2 3 4 5 6 7 8 9'
    num_screens = 5
    deploy_screens(num_screens, move_str)
    # wait 10 minutes before exit
    sleep(60 * 10)
    kill_screens()
    # verify results
    raw_psql_table = get_raw_psql_table_output(identity_file)
    (psql_table_headers, psql_table_data) = \
            tp.get_psql_table_data(raw_psql_table)
    result = verify_wins_losses(num_screens, psql_table_data)
    cleanup(identity_file)
    return result


def main(args):
    """
    arg 1: Java client working directory
    arg 2: SSH private key file
    """
    global WORKING_DIR
    WORKING_DIR = args.working_dir
    result = default_test(args.identity_file)
    if not result:
        print 'Tests failed.'
    else:
        print 'Tests passed.'


parser = argparse.ArgumentParser()
parser.add_argument("working_dir", help="Java client working directory",
                    type=str)
parser.add_argument("identity_file", help="SSH private key (identity) file",
                    type=str)
args = parser.parse_args()
main(args)
