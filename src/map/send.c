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
#include "../../../map/mob.h"
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

void send_changelook(int fd, int id, int type, int val)
{
    WFIFOHEAD (fd, 11);
    WFIFOW (fd, 0) = 0x1d7;
    WFIFOL (fd, 2) = id;
    WFIFOB (fd, 6) = type;
    WFIFOW (fd, 7) = val;
    WFIFOW (fd, 9) = 0;
    WFIFOSET (fd, 11);
}

void send_mapmask(int fd, int mask)
{
    WFIFOHEAD (fd, 10);
    WFIFOW (fd, 0) = 0xb02;
    WFIFOL (fd, 2) = mask;
    WFIFOL (fd, 6) = 0;
    WFIFOSET (fd, 10);
}

void send_mapmask_brodcast(const int map, const int mask)
{
    struct block_list bl;
    char buf[10];

    bl.m = map;
    WBUFW (buf, 0) = 0xb02;
    WBUFL (buf, 2) = mask;
    WBUFL (buf, 6) = 0;
    clif->send(buf, 10, &bl, ALL_SAMEMAP);
}

void send_mob_info(struct block_list* bl1, struct block_list* bl2,
                   enum send_target target)
{
    char buf[12];

    if (bl1->type != BL_MOB)
        return;

    struct mob_data *md = (struct mob_data *)bl1;

    WBUFW (buf, 0) = 0xb03;
    WBUFW (buf, 2) = 12; // len
    WBUFL (buf, 4) = md->bl.id;
    WBUFL (buf, 8) = md->status.rhw.range;

    clif->send(&buf, sizeof(buf), bl2, target);
}
