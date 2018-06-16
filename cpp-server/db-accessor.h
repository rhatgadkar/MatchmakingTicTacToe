#ifndef DB_ACCESSOR_H
#define DB_ACCESSOR_H

void get_login_info(const char *login_str, char *username, char *password);
int is_login_valid(char *username, char *password);
void set_user_no_ingame(const char *username);
void get_win_loss_record(const char *username, char *win, char *loss);
void update_win_loss_record(const char *username1, char rec1,
		const char *username2, char rec2);
int get_port_pop(int port);
void set_port_pop(int port, int pop);
int get_total_pop();

#endif  // DB_ACCESSOR_H
