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
#include "../../../map/pc.h"

#include "map/send.h"

void send_npccommand (struct map_session_data *sd, int npcId, int cmd)
{
    if (!sd)
        return;

    int  fd = sd->fd;
    WFIFOHEAD (fd, 16);
    WFIFOW (fd, 0) = 0xB00;
    WFIFOL (fd, 2) = npcId;
    WFIFOW (fd, 6) = cmd;
    WFIFOL (fd, 8) = 0;
    WFIFOW (fd, 12) = 0;
    WFIFOW (fd, 14) = 0;
    WFIFOSET (fd, 16);
}

// 0 - get client lang
void send_npccommand2 (struct map_session_data *sd, int npcId, int cmd, int id, int x, int y)
{
    if (!sd)
        return;

    int  fd = sd->fd;
    WFIFOHEAD (fd, 16);
    WFIFOW (fd, 0) = 0xB00;
    WFIFOL (fd, 2) = npcId;
    WFIFOW (fd, 6) = cmd;
    WFIFOL (fd, 8) = id;
    WFIFOW (fd, 12) = x;
    WFIFOW (fd, 14) = y;
    WFIFOSET (fd, 16);
}

void send_local_message(int fd, struct block_list* bl, const char* msg)
{
    unsigned short msg_len = strlen(msg) + 1;
    uint8 buf[256];
    if (!bl)
        return;

    int len = sizeof(buf) - 8;
    if (msg_len > len)
    {
        ShowWarning("clif_message: Truncating too long message '%s' (len=%u).\n", msg, msg_len);
        msg_len = len;
    }

    WFIFOHEAD (fd, msg_len + 8);
    WFIFOW (fd, 0) = 0x8d;
    WFIFOW (fd, 2) = msg_len + 8;
    WFIFOL (fd, 4) = bl->id;
    safestrncpy((char*)WFIFOP(fd, 8), msg, msg_len);
    WFIFOSET (fd, msg_len + 8);
}
