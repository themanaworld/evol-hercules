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
#include "../../../char/char.h"

#include "char/char.h"
#include "char/config.h"

void echar_parse_char_login_map_server(int *fd)
{
    if (!inter_server_ip)
        return;

    const uint32 ipl = session[*fd]->client_addr;

    const char *const ip = ip2str(ipl, NULL);
    if (!strstr(inter_server_ip, ip))
    {
        hookStop();
        ShowNotice("Connection of the map-server from ip %s REFUSED.\n", ip);
        chr->login_map_server_ack(*fd, 3);
    }
}
