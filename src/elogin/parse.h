// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_LOGIN_PARSE
#define EVOL_LOGIN_PARSE

void login_parse_version(int fd);
bool elogin_client_login_pre(int *fd,
                             struct login_session_data **sdPtr);
bool elogin_client_login_post(bool retVal,
                              int fd,
                              struct login_session_data *sd);
void elogin_parse_client_login2(int fd);
enum parsefunc_rcode elogin_parse_ping_pre(int *fd,
                                           struct login_session_data **sdPtr);
void elogin_parse_change_paassword(int fd);

#endif  // EVOL_LOGIN_PARSE
