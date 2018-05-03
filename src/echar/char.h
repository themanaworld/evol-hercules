// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_CHAR_CHAR
#define EVOL_CHAR_CHAR

void echar_parse_char_create_new_char(int *fdPtr,
                                      struct char_session_data **sdPtr);

void echar_creation_failed(int *fdPtr, int *result);

void echar_parse_change_paassword(int fd);

void echar_parse_login_password_change_ack(int charFd);

void echar_mmo_char_send099d_post(int fd, struct char_session_data *sd);

int echar_mmo_char_send_characters_post(int retVal, int fd, struct char_session_data *sd);

void send_additional_slots(int fd, struct char_session_data* sd);

void echar_parse_char_connect_pre(int *fdPtr, struct char_session_data **sd, uint32 *ipl);

void echar_parse_char_connect_post(int fd, struct char_session_data *sd, uint32 ipl);

void echar_parse_map_serverexit(int mapFd);

void echat_send_login_serverexit(const int code);

#endif  // EVOL_CHAR_CHAR
