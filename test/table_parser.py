import commands


PSQL_CMD = "psql -d mydb -c 'SELECT * FROM tttrecords;'"
PS_CMD = "ps ww"


def get_table_data(input_str, pop_before_data_start=0,
                   pop_after_data_end=0, sep=' '):
    table_rows = input_str.split('\n')
    table_headers = table_rows[0].split(sep)
    table_headers = [item for item in table_headers if item != '']
    # strip whitespace surrounded elements in table_headers
    for j in range(0, len(table_headers)):
        table_headers[j] = table_headers[j].strip()
    # pop headers
    table_rows.pop(0)
    # pop before data start
    for i in range(0, pop_before_data_start):
        table_rows.pop(0)
    # pop after data end
    for i in range(0, pop_after_data_end):
        table_rows.pop(-1)
    for i in range(0, len(table_rows)):
        table_rows[i] = table_rows[i].split(sep)
        table_rows[i] = [item for item in table_rows[i] if item != '']
        # make len(table_rows[i]) == len(table_headers)
        to_append = []
        while len(table_rows[i]) > len(table_headers):
            to_append.append(table_rows[i].pop(-1))
        while len(to_append) > 0:
            str_append = ' ' + to_append.pop(-1)
            table_rows[i][len(table_headers)-1] += str_append
        # strip whitespace surrounded elements in table_rows[i]
        for j in range(0, len(table_rows[i])):
            table_rows[i][j] = table_rows[i][j].strip()
    return (table_headers, table_rows)


def get_psql_table_data(input_str):
    return get_table_data(input_str, 1, 2, '|')


def get_ps_table_data(input_str):
    return get_table_data(input_str)


import argparse
parser = argparse.ArgumentParser()
parser.add_argument("identity_file", help="SSH private key (identity) file",
                    type=str)
parser.add_argument("server_addr", help="Server address to SSH into", type=str)
args = parser.parse_args()
ssh_cmd = 'ssh -i ' + args.identity_file + ' ' + args.server_addr
get_tttrecords_cmd = ssh_cmd + ' "' + PSQL_CMD + '"'
raw_data = commands.getoutput(get_tttrecords_cmd)
(table_headers, table_rows) = get_psql_table_data(raw_data)
print table_headers
print table_rows
raw_data = commands.getoutput(PS_CMD)
(table_header, table_rows) = get_ps_table_data(raw_data)
print table_header
print table_rows
