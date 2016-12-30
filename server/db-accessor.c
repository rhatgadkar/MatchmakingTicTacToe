#define CMD_SIZE 1000
#define VAR_SIZE 80

#include <libpq-fe.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "db-accessor.h"

static PGconn * db_connect(const char *conninfo);
static PGresult * exec_command(PGconn *conn, const char *cmd, int get_data);

void
get_login_info(const char *login_str, char *username, char *password)
{
	char *pch;
	int sep_pos;

	pch = strchr(login_str, ',');
	sep_pos = pch - login_str;
	strncpy(username, login_str, sep_pos);
	strncpy(password, login_str + sep_pos + 1,
			strlen(login_str) - sep_pos - 1);
}

int
is_login_valid(char *username, char *password)
{
	PGconn *conn;
	int status;
	char cmd_str[CMD_SIZE];
	PGresult *res;
	int output_null;

	conn = db_connect("dbname=mydb");
	memset(cmd_str, 0, CMD_SIZE);

	// check if user exists
	status = snprintf(cmd_str, CMD_SIZE,
			"SELECT * \
			FROM tttlogin \
			WHERE tttlogin.username='%s'",
			username);
	if (status < 0)
	{
		fprintf(stderr, "snprintf failed\n");
		PQfinish(conn);
		return 0;
	}
	res = exec_command(conn, cmd_str, 1);
	output_null = PQgetisnull(res, 0, 0);
	if (!output_null)
	{
		// user exists - check if passwd matches
		if (strcmp(PQgetvalue(res, 0, 1), password) != 0)
		{
			printf("User exists. password doesn't match.\n");
			PQclear(res);
			PQfinish(conn);
			return 0;
		}
		if (strcmp(PQgetvalue(res, 0, 2), "f") != 0)
		{
			printf("User is currently in game.\n");
			PQclear(res);
			PQfinish(conn);
			return -1;
		}
		PQclear(res);
		// set ingame to 't'
		status = snprintf(cmd_str, CMD_SIZE,
				"UPDATE tttlogin \
				SET ingame=TRUE \
				WHERE tttlogin.username='%s'",
				username);
		if (status < 0)
		{
			fprintf(stderr, "snprintf failed\n");
			PQfinish(conn);
			return 0;
		}
		res = exec_command(conn, cmd_str, 0);

		PQfinish(conn);
		return 1;
	}
	else
	{
		PQclear(res);
		// add new user to tttlogin and tttrecords
		printf("Adding new user.\n");
		status = snprintf(cmd_str, CMD_SIZE,
				"INSERT INTO tttlogin (username,password,ingame)\
				VALUES ('%s','%s',TRUE)",
				username, password);
		if (status < 0)
		{
			fprintf(stderr, "snprintf failed\n");
			PQfinish(conn);
			return 0;
		}
		res = exec_command(conn, cmd_str, 0);
		PQclear(res);
		memset(cmd_str, 0, CMD_SIZE);
		status = snprintf(cmd_str, CMD_SIZE,
				"INSERT INTO tttrecords (username,win,loss)\
				VALUES ('%s',0,0)",
				username);
		if (status < 0)
		{
			fprintf(stderr, "snprintf failed\n");
			PQfinish(conn);
			return 0;
		}
		res = exec_command(conn, cmd_str, 0);
		PQclear(res);
		PQfinish(conn);
		return 1;
	}
	PQfinish(conn);
	return 0;
}

void
set_user_no_ingame(char *username)
{
	PGconn *conn;
	int status;
	char cmd_str[CMD_SIZE];
	PGresult *res;

	conn = db_connect("dbname=mydb");
	memset(cmd_str, 0, CMD_SIZE);

	status = snprintf(cmd_str, CMD_SIZE,
			"UPDATE tttlogin \
			SET ingame=FALSE \
			WHERE tttlogin.username='%s'",
			username);
	if (status < 0)
	{
		fprintf(stderr, "snprintf failed\n");
		PQfinish(conn);
		return;
	}
	res = exec_command(conn, cmd_str, 0);
	PQfinish(conn);
}

void
get_win_loss_record(char *username, char *win, char *loss)
{
	PGconn *conn;
	int status;
	char cmd_str[CMD_SIZE];
	PGresult *res;

	conn = db_connect("dbname=mydb");
	memset(cmd_str, 0, CMD_SIZE);

	// get win
	status = snprintf(cmd_str, CMD_SIZE,
			"SELECT tttrecords.win \
			FROM tttrecords \
			WHERE tttrecords.username='%s'",
			username);
	if (status < 0)
	{
		fprintf(stderr, "snprintf failed\n");
		PQfinish(conn);
		exit(1);
	}
	res = exec_command(conn, cmd_str, 1);
	strcpy(win, PQgetvalue(res, 0, 0));
	PQclear(res);
	// get loss
	status = snprintf(cmd_str, CMD_SIZE,
			"SELECT tttrecords.loss \
			FROM tttrecords \
			WHERE tttrecords.username='%s'",
			username);
	if (status < 0)
	{
		fprintf(stderr, "snprintf failed\n");
		PQfinish(conn);
		exit(1);
	}
	res = exec_command(conn, cmd_str, 1);
	strcpy(loss, PQgetvalue(res, 0, 0));

	PQfinish(conn);
}

void
update_win_loss_record(char *username1, char rec1, char *username2, char rec2)
{
	PGconn *conn;
	int status;
	char cmd_str[CMD_SIZE];
	PGresult *res;

	conn = db_connect("dbname=mydb");
	memset(cmd_str, 0, CMD_SIZE);

	if (rec1 == 'l')
	{
		status = snprintf(cmd_str, CMD_SIZE,
				"UPDATE tttrecords \
				SET loss=loss+1 \
				WHERE tttrecords.username='%s'",
				username1);
		if (status < 0)
		{
			fprintf(stderr, "snprintf failed\n");
			PQfinish(conn);
			exit(1);
		}
		res = exec_command(conn, cmd_str, 0);
		PQclear(res);
		memset(cmd_str, 0, CMD_SIZE);
	}
	else
	{
		status = snprintf(cmd_str, CMD_SIZE,
				"UPDATE tttrecords \
				SET win=win+1 \
				WHERE tttrecords.username='%s'",
				username1);
		if (status < 0)
		{
			fprintf(stderr, "snprintf failed\n");
			PQfinish(conn);
			exit(1);
		}
		res = exec_command(conn, cmd_str, 0);
		PQclear(res);
		memset(cmd_str, 0, CMD_SIZE);
	}

	if (rec2 == 'l')
	{
		status = snprintf(cmd_str, CMD_SIZE,
				"UPDATE tttrecords \
				SET loss=loss+1 \
				WHERE tttrecords.username='%s'",
				username2);
		if (status < 0)
		{
			fprintf(stderr, "snprintf failed\n");
			PQfinish(conn);
			exit(1);
		}
		res = exec_command(conn, cmd_str, 0);
		PQclear(res);
		memset(cmd_str, 0, CMD_SIZE);
	}
	else
	{
		status = snprintf(cmd_str, CMD_SIZE,
				"UPDATE tttrecords \
				SET win=win+1 \
				WHERE tttrecords.username='%s'",
				username2);
		if (status < 0)
		{
			fprintf(stderr, "snprintf failed\n");
			PQfinish(conn);
			exit(1);
		}
		res = exec_command(conn, cmd_str, 0);
		PQclear(res);
		memset(cmd_str, 0, CMD_SIZE);
	}

	PQfinish(conn);
}

PGconn *
db_connect(const char *conninfo)
{
	PGconn *conn;

	conn = PQconnectdb(conninfo);
	if (PQstatus(conn) != CONNECTION_OK)
	{
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(conn));
		PQfinish(conn);
		exit(1);
	}
	return conn;
}

PGresult *
exec_command(PGconn *conn, const char *cmd, int get_data)
{
	PGresult *res;
	ExecStatusType st;

	if (get_data)
		st = PGRES_TUPLES_OK;
	else
		st = PGRES_COMMAND_OK;
	res = PQexec(conn, cmd);
	if (PQresultStatus(res) != st)
	{
		fprintf(stderr, "'%s' failed: %s",
				cmd, PQerrorMessage(conn));
		PQclear(res);
		PQfinish(conn);
		exit(1);
	}
	return res;
}
