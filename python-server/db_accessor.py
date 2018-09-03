import psycopg2

def get_login_info(login_str):
    login_list = login_str.split(',')
    username = login_list[0]
    password = login_list[1]
    return (username, password)

def is_login_valid(username, password):
    conn = psycopg2.connect("dbname=mydb")
    cur = conn.cursor()
    cmd_str = "SELECT * FROM tttlogin WHERE tttlogin.username=%s"
    data = (username, )
    cur.execute(cmd_str, data)
    result = cur.fetchone()
    if result:
        # user exists in db
        db_username = result[0]
        db_password = result[1]
        db_ingame = result[2]
        if password != db_password:
            print "User exists.  password doesn't match."
            cur.close()
            conn.close()
            return 0
        if db_ingame == 'f':
            print "User is currently in game."
            cur.close()
            conn.close()
            return -1
        cmd_str = "UPDATE tttlogin SET ingame=TRUE WHERE tttlogin.username=%s"
        data = (username, )
        cur.execute(cmd_str, data)
        conn.commit()
        cur.close()
        conn.close()
        return 1
    else:
        # add user to db
        cmd_str = "INSERT INTO tttlogin (username,password,ingame) VALUES (%s,%s,TRUE)"
        data = (username, password)
        cur.execute(cmd_str, data)
        cmd_str = "INSERT INTO tttrecords (username,win,loss) VALUES (%s,0,0)"
        data = (username, )
        cur.execute(cmd_str, data)
        conn.commit()
        cur.close()
        conn.close()
        return 1
    return 0
