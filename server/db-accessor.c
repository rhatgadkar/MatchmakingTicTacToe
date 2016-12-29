#define CMD_SIZE 1000
#define VAR_SIZE 80

#include <libpq-fe.h>
#include <stdio.h>
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
		PQclear(res);
		PQfinish(conn);
		return 1;
	}
	PQclear(res);
	PQfinish(conn);
	return 0;
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
