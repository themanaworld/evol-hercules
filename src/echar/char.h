// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_CHAR_CHAR
#define EVOL_CHAR_CHAR

void echar_parse_char_create_new_char(int *fdPtr, struct char_session_data* sd);

void echar_creation_failed(int *fdPtr, int *result);

void echar_parse_char_ping(int *fdPtr);

void echar_parse_change_paassword(int fd);

void echar_parse_login_password_change_ack(int charFd);

void echar_mmo_char_send099d_post(int *fdPtr, struct char_session_data *sd);

int echar_mmo_char_send_characters_post(int retVal, int *fdPtr, struct char_session_data *sd);

int echar_mmo_chars_fromsql(struct char_session_data* sd, uint8* buf);

void echar_mmo_char_send099d(int *fdPtr, struct char_session_data *sd);

void send_additional_slots(int fd, struct char_session_data* sd);

void echar_parse_char_connect_pre(int *fdPtr, struct char_session_data *sd, uint32 *ipl);

void echar_parse_char_connect_post(int *fdPtr, struct char_session_data *sd, uint32 *ipl);

void echar_parse_frommap_request_stats_report_pre(int *fdPtr);

#endif  // EVOL_CHAR_CHAR
