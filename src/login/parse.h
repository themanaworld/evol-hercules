// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_LOGIN_PARSE
#define EVOL_LOGIN_PARSE

void login_parse_version(int fd);
int elogin_parse_client_login_pre(int *fd, struct login_session_data* sd, const char *const ip);
void elogin_parse_client_login2(int fd);
void elogin_parse_request_connection(int *fd, struct login_session_data* sd, const char *const ip);
void elogin_parse_ping(int *fd, struct login_session_data* sd);

#endif  // EVOL_LOGIN_PARSE
