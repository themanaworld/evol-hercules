// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_LOGIN_PARSE
#define EVOL_LOGIN_PARSE

void login_parse_version(int fd);
int elogin_client_login_pre(int *fd, struct login_session_data* sd);
int elogin_client_login_post(int retVal, int *fd, struct login_session_data* sd);
void elogin_parse_client_login2(int fd);
void elogin_parse_ping(int *fd, struct login_session_data* sd);
void elogin_parse_change_paassword(int fd);

#endif  // EVOL_LOGIN_PARSE
