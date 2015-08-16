// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "login/login.h"

#include "elogin/config.h"
#include "elogin/send.h"

void send_server_version(int fd)
{
    WFIFOHEAD(fd, 4 + 8);
    WFIFOW(fd, 0) = 0x7531;
    WFIFOW(fd, 2) = 4 + 8;
    WFIFOL(fd, 4) = 0;  // unused
    WFIFOL(fd, 8) = 7;  // server version
    WFIFOSET(fd, WFIFOW(fd,2));
}

void send_update_host(int fd)
{
    if (!update_server)
        return;
    const int sz = strlen(update_server);
    WFIFOHEAD(fd, sz + 4);
    WFIFOW(fd, 0) = 0x63;
    WFIFOW(fd, 2) = sz + 4;
    memcpy(WFIFOP (fd, 4), update_server, sz);
    WFIFOSET(fd, sz + 4);
}

void send_char_password_change_ack(int fd, int accoundId, char status)
{
    WFIFOHEAD(fd, 7);
    WFIFOW(fd, 0) = 0x5001;
    WFIFOL(fd, 2) = accoundId;
    WFIFOB(fd, 6) = status;
    WFIFOSET(fd, 7);
}