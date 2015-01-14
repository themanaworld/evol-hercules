// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_CHAR_CHAR
#define EVOL_CHAR_CHAR

void echar_parse_char_login_map_server(int *fd);

void echar_parse_char_create_new_char(int *fdPtr, struct char_session_data* sd);

void echar_creation_failed(int *fdPtr, int *result);

#endif  // EVOL_CHAR_CHAR
