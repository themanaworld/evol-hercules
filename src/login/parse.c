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

#include "login/parse.h"

int clientVersion = 0;

void login_parse_version(int fd)
{
    struct login_session_data* sd = (struct login_session_data*)session[fd]->session_data;
    if (!sd)
        return;

    clientVersion = RFIFOL(fd, 2);

    if (clientVersion < 2)
    {
        login->login_error(fd, 5);
        return;
    }

    WFIFOHEAD(fd, 4 + 8);
    WFIFOW(fd, 0) = 0x7531;
    WFIFOW(fd, 2) = 4 + 8;
    WFIFOL(fd, 4) = 0;  // unused
    WFIFOL(fd, 8) = 1;  // server version
    WFIFOSET(fd, WFIFOW(fd,2));
}

int elogin_parse_client_login_pre(int *fd, struct login_session_data* sd, const char *const ip)
{
    if (clientVersion < 2)
    {
        login->login_error(*fd, 5);
        hookStop();
        return 1;
    }
    return 0;
}
