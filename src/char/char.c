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
    uint16 race = 0;
    uint16 look = 0;
    uint8 sex = 0;

    if (sd->version >= 4)
    {
        race = RFIFOW(fd, 31);
        if (race < min_char_class || race > max_char_class)
        {
            chr->creation_failed(fd, -10);
            RFIFOSKIP(fd, 31 + 5);
            hookStop();
            return;
        }
        sex = RFIFOB(fd, 33);
        if (sex > 1 && sex != 99)
        {
            chr->creation_failed(fd, -11);
            RFIFOSKIP(fd, 31 + 5);
            hookStop();
            return;
        }
        look = RFIFOW(fd, 34);
    }

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

        if (sd->version >= 4)
        {
            char_dat.class_ = race;
            char_dat.sex = sex;
            char_dat.clothes_color = look;

            chr->mmo_char_tosql(result, &char_dat);
        }
        chr->creation_ok(fd, &char_dat);

        // add new entry to the chars list
        sd->found_char[char_dat.slot] = result; // the char_id of the new char
    }
    if (sd->version >= 4)
        RFIFOSKIP(fd, 31 + 5);
    else
        RFIFOSKIP(fd, 31);
    hookStop();
}

void echar_creation_failed(int *fdPtr, int *result)
{
    const int fd = *fdPtr;
    WFIFOHEAD(fd, 3);
    WFIFOW(fd, 0) = 0x6e;
    /* Others I found [Ind] */
    /* 0x02 = Symbols in Character Names are forbidden */
    /* 0x03 = You are not eligible to open the Character Slot. */
    /* 0x0B = This service is only available for premium users.  */
    switch (*result)
    {
        case -1: WFIFOB(fd, 2) = 0x00; break; // 'Charname already exists'
        case -2: WFIFOB(fd, 2) = 0xFF; break; // 'Char creation denied'
        case -3: WFIFOB(fd, 2) = 0x01; break; // 'You are underaged'
        case -4: WFIFOB(fd, 2) = 0x03; break; // 'You are not eligible to open the Character Slot.'
        case -5: WFIFOB(fd, 2) = 0x02; break; // 'Symbols in Character Names are forbidden'
        case -10: WFIFOB(fd, 2) = 0x50; break; // Wrong class
        case -11: WFIFOB(fd, 2) = 0x51; break; // Wrong sex

        default:
            ShowWarning("chr->parse_char: Unknown result received from chr->make_new_char_sql: %d!\n", *result);
            WFIFOB(fd,2) = 0xFF;
            break;
    }
    WFIFOSET(fd,3);
    hookStop();
}
