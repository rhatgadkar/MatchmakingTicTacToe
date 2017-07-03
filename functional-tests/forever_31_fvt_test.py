import argparse
import fvt_tests
from time import sleep
import traceback


def forever_31_clients_test():
    """
    31 clients are running forever with move set of '1 2 3 4 5 6 7 8 9'.
    """
    move_str = '1 2 3 4 5 6 7 8 9'
    num_screens = 31
    while True:
	    try:
		fvt_tests.deploy_screens(num_screens, move_str, False)
	    except:
		print 'Exception occurred.'
		traceback.print_exc()
		return False
    return True


def main(args):
    """
    This is the entry point for running the forever_31_clients_test.

    arg 1: Java client working directory
    arg 2: SSH private key file
    """
    fvt_tests.WORKING_DIR = args.working_dir
    fvt_tests.IDENTITY_FILE = args.identity_file
    fvt_tests.STARTING_USER_NUM = args.start_user_num
    result = forever_31_clients_test()
    if not result:
        print 'forever_31_clients_test exited.'
        return


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
