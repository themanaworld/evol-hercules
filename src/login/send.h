// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_LOGIN_SEND
#define EVOL_LOGIN_SEND

void send_server_version();
void send_update_host(int fd);
void send_char_password_change_ack(int fd, int accoundId, char status);

#endif  // EVOL_LOGIN_SEND
