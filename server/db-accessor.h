#ifndef DB_ACCESSOR_H
#define DB_ACCESSOR_H

void get_login_info(const char *login_str, char *username, char *password);
int is_login_valid(char *username, char *password);
void set_user_no_ingame(char *username);

#endif  // DB_ACCESSOR_H
