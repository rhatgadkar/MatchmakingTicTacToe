def get_table_data(input_str, pop_before_data_start=0,
                   pop_after_data_end=0, sep=' '):
    """
    Returns a table header and its data given from an input string.

    Input                  Description                                     Type
    ---------------------------------------------------------------------------
    input_str              The raw table output                             str
    pop_before_data_start  Number of lines to pop from input_str before     int
                           data rows begin after getting table header
    pop_after_data_end     Number of lines to pop from input_str after      int
                           having gotten the data rows from the table
    sep                    Symbol that is used to separate the colums of    str
                           the table

    Example PSQL table output:
        username | win | loss 
        ----------+-----+------
         a        |   3 |    3
         b        |   3 |    3
         s1       |   6 |    2
        (3 rows)

        get_table_data(...) arguments:
        pop_before_data_start=1, pop_after_data_end=2, sep='|'

        Output of get_table_data(...) with provided arguments:
        ['username', 'win', 'loss']
        [['a', '3', '3'], ['b', '3', '3'], ['s1', '6', '2']]

    Example 'ps ww' table output:
         PID TTY      STAT   TIME COMMAND
        2274 pts/6    Ss     0:00 /bin/bash
        2723 pts/6    S+     0:00 python table_parser.py aws-ubuntu1404.pem
        2727 pts/6    S+     0:00 sh -c { ps ww; } 2>&1
        2728 pts/6    R+     0:00 ps ww

        get_table_data(...) arguments:
        pop_before_data_start=0, pop_after_data_end=0, sep=' '

        Output of get_table_data(...) with provided arguments:
        ['PID', 'TTY', 'STAT', 'TIME', 'COMMAND']
        [['2274', 'pts/6', 'Ss', '0:00', '/bin/bash'], ['2723', 'pts/6', 'S+', '0:00', 'python table_parser.py aws-ubuntu1404.pem'], ['2727', 'pts/6', 'S+', '0:00', 'sh -c { ps ww; } 2>&1'], ['2728', 'pts/6', 'R+', '0:00', 'ps ww']]
    """
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
