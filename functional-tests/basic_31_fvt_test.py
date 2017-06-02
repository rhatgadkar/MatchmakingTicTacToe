import argparse
import fvt_tests
from time import sleep
import traceback


def basic_31_clients_test():
    """
    31 clients are running for 30 minutes with move set of '1 2 3 4 5 6 7 8 9'.
    """
    result = False
    move_str = '1 2 3 4 5 6 7 8 9'
    num_screens = 31
    try:
        fvt_tests.deploy_screens(num_screens, move_str)
        # wait 30 minutes before exit
        sleep(60 * 30)
        fvt_tests.kill_screens()
        # verify results of screen logs with psql table output
        result = fvt_tests.verify_wins_losses()
        if not result:
            return False
    except:
        print 'Exception occurred.'
        traceback.print_exc()
        return False
    fvt_tests.get_ports_usage()
    return result


def main(args):
    """
    This is the entry point for running the functional tests.

    arg 1: Java client working directory
    arg 2: SSH private key file
    """
    fvt_tests.WORKING_DIR = args.working_dir
    fvt_tests.IDENTITY_FILE = args.identity_file
    fvt_tests.STARTING_USER_NUM = args.start_user_num
    result = basic_31_clients_test()
    if not result:
        print 'basic_31_clients_test failed.'
        return
    print 'All tests passed.'


if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument("working_dir", help="Java client working directory",
						type=str)
	parser.add_argument("identity_file",
						help="SSH private key (identity) file", type=str)
	# if start_user_num is '1', it will begin deploying screens from 's1'
	parser.add_argument("start_user_num", help="The starting num of the user",
						type=int)
	args = parser.parse_args()
	main(args)
