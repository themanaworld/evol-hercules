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

#include "common/ip.h"
#include "char/char.h"
#include "char/config.h"

void echar_parse_char_login_map_server(int *fd)
{
    if (!inter_server_ip)
        return;

    const uint32 ipl = session[*fd]->client_addr;

    const char *const ip = ip2str(ipl, NULL);
    if (!checkAllowedIp(inter_server_ip, ip))
    {
        ShowNotice("Connection of the map-server from ip %s REFUSED.\n", ip);
        chr->login_map_server_ack(*fd, 3);
        hookStop();
    }
}

void echar_parse_char_create_new_char(int *fdPtr, struct char_session_data* sd)
{
    // ignore char creation disable option
    const int fd = *fdPtr;
    const int result = chr->make_new_char_sql(sd, (char*)RFIFOP(fd, 2), 1, 1, 1, 1, 1, 1, RFIFOB(fd, 26), RFIFOW(fd, 27), RFIFOW(fd, 29));
    if (result < 0)
    {
        chr->creation_failed(fd, result);
    }
    else
    {
        // retrieve data
        struct mmo_charstatus char_dat;
        chr->mmo_char_fromsql(result, &char_dat, false); //Only the short data is needed.

        const uint16 race = RFIFOW(fd, 31);
        char_dat.class_ = race;
        chr->mmo_char_tosql(result, &char_dat);

        chr->creation_ok(fd, &char_dat);

        // add new entry to the chars list
        sd->found_char[char_dat.slot] = result; // the char_id of the new char
    }
    RFIFOSKIP(fd, 31 + 2);
    hookStop();
}
