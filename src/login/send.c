// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../common/HPMi.h"
#include "../../../common/malloc.h"
#include "../../../common/mmo.h"
#include "../../../common/socket.h"
#include "../../../common/strlib.h"
#include "../../../login/login.h"

#include "login/config.h"
#include "login/send.h"

void send_server_version(int fd)
{
    WFIFOHEAD(fd, 4 + 8);
    WFIFOW(fd, 0) = 0x7531;
    WFIFOW(fd, 2) = 4 + 8;
    WFIFOL(fd, 4) = 0;  // unused
    WFIFOL(fd, 8) = 3;  // server version
    WFIFOSET(fd, WFIFOW(fd,2));
}

void send_update_host(int fd)
{
    if (!update_server)
        return;
    const int sz = strlen(update_server);
    WFIFOHEAD(fd, sz);
    WFIFOW(fd, 0) = 0x63;
    WFIFOW(fd, 2) = sz + 4;
    memcpy(WFIFOP (fd, 4), update_server, sz);
    WFIFOSET(fd, sz + 4);
}
