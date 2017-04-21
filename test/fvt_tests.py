import argparse
from screenutils import Screen
import table_parser as tp
import commands


#WORKING_DIR = ''
WORKING_DIR = '/home/rishabh/Documents/MatchmakingTicTacToe/java-client/src'
DEPLOY_SCREENS_CMD = 'python run_java_test.py'
PSQL_PREFIX_CMD = "psql -d mydb -c "
PSQL_SELECT_CMD = PSQL_PREFIX_CMD + "'SELECT * FROM tttrecords;'"
PSQL_DELETE_CMD = PSQL_PREFIX_CMD + \
    "'DELETE * FROM tttrecords WHERE SUBSTR(username, 1, 1) = 's';'"
PS_CMD = "ps ww"
SERVER_ADDR = "ubuntu@54.219.156.253"


def get_raw_psql_table_output(identity_file):
    ssh_cmd = 'ssh -i ' + args.identity_file + ' ' + SERVER_ADDR
    get_tttrecords_cmd = ssh_cmd + ' "' + PSQL_SELECT_CMD + '"'
    raw_data = commands.getoutput(get_tttrecords_cmd)
    return raw_data


def get_raw_ps_table_output():
    return commands.getoutpu(PS_CMD)


def deploy_screens(num_screens, move_str):
    for i in range(0, num_screens):
        scr_name = 's' + str(i + 1)
        fvt_args = WORKING_DIR + ' ' + scr_name + ' ' + move_str
        s = Screen(scr_name, True)
        s.enable_logs(scr_name + '.log')
        s.send_commands(DEPLOY_SCREENS_CMD + ' ' + fvt_args)


# tests:
def default_test(identity_file):
    move_str = '1 2 3 4 5 6 7 8 9'
    num_screens = 5
    deploy_screens(num_screens, move_str)
    # wait before exit
    # send exit command
    # verify results
    raw_psql_table = get_raw_psql_table_output(identity_file)
    (psql_table_headers, psql_table_data) = get_psql_table_data(raw_psql_table)
    


def main(args):
    """
    arg 1: Java client working directory
    arg 2: SSH private key file
    """
    WORKING_DIR = args.working_dir
    default_test(args.identity_file)


#parser = argparse.ArgumentParser()
#parser.add_argument("working_dir", help="Java client working directory",
#                    type=str)
#parser.add_argument("identity_file", help="SSH private key (identity) file",
#                    type=str
#args = parser.parse_args()
#main(args)
