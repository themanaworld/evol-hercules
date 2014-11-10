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
#include "login/send.h"

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

    send_update_host(fd);
    send_server_version(fd);
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
