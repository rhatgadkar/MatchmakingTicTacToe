#ifndef DB_ACCESSOR_H
#define DB_ACCESSOR_H

void get_login_info(const char *login_str, char *username, char *password);
int is_login_valid(char *username, char *password);

#endif  // DB_ACCESSOR_H
