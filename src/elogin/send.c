// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "login/login.h"

#include "ecommon/serverversion.h"
#include "elogin/config.h"
#include "elogin/send.h"

void send_server_version(int fd)
{
    const int sz = 20;
    WFIFOHEAD(fd, sz);
    WFIFOW(fd, 0) = 0x7531;
    WFIFOW(fd, 2) = sz;
    WFIFOL(fd, 4) = 0;   // unused
    WFIFOL(fd, 8) = 20;  // plugin version
    WFIFOL(fd, 12) = serverPacketVersion;  // server packet version
    WFIFOW(fd, 16) = evolPacketOffset;     // packet id offset
    int flags = 0;
#ifdef PACKETVER_RE
    flag |= 1;
#endif
#ifdef PACKETVER_ZERO
    flag |= 2;
#endif
    WFIFOW(fd, 18) = flags;

    WFIFOSET(fd, WFIFOW(fd,2));
}

void send_update_host(int fd)
{
    if (!update_server)
        return;
    const int sz = (int)strlen(update_server);
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
