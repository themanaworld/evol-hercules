// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/clif.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/pet.h"
#include "map/unit.h"

#include "emap/send.h"
#include "emap/permission.h"
#include "emap/data/session.h"
#include "emap/struct/sessionext.h"

void send_npccommand (TBL_PC *sd, int npcId, int cmd)
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
void send_npccommand2 (TBL_PC *sd, int npcId, int cmd, int id, int x, int y)
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
    if (!msg || !bl)
        return;
    unsigned short msg_len = strlen(msg) + 1;
    uint8 buf[256];

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

void send_changelook(struct map_session_data* sd, struct map_session_data* sd2, int fd,
                     int id, int type, int val, int val2,
                     struct item_data *data, int n)
{
    struct SessionExt *tdata = session_get_bysd(sd2);
    int i;
    //ShowWarning("equip: for type %d = %d\n", type, val);
    if (!tdata || tdata->clientVersion < 9)
    {
        WFIFOHEAD (fd, 11);
        WFIFOW (fd, 0) = 0x1d7;
        WFIFOL (fd, 2) = id;
        WFIFOB (fd, 6) = type;
        WFIFOW (fd, 7) = val;
        WFIFOW (fd, 9) = val2;
        WFIFOSET (fd, 11);
    }
    else
    {
        WFIFOHEAD (fd, 19);
        WFIFOW (fd, 0) = 0xb17;
        WFIFOL (fd, 2) = id;
        WFIFOB (fd, 6) = type;
        WFIFOW (fd, 7) = val;
        WFIFOW (fd, 9) = val2;
        if (data)
        {
            for (i = 0; i < data->slot; i++ )
            {
                struct item_data *data;
                if (!sd->status.inventory[n].card[i])
                    continue;
                if ((data = itemdb->exists(sd->status.inventory[n].card[i])) != NULL)
                {
                    //ShowWarning("card %d\n", data->nameid);
                    WFIFOW (fd, 11 + i * 2) = data->nameid;
                }
            }
            for (i = data->slot; i < MAX_SLOTS; i ++)
                WFIFOW (fd, 11 + i * 2) = 0;
        }
        else
        {
            //ShowWarning("unequip: for type %d\n", type);
            WFIFOW (fd, 11) = 0;
            WFIFOW (fd, 13) = 0;
            WFIFOW (fd, 15) = 0;
            WFIFOW (fd, 17) = 0;
        }
        WFIFOSET (fd, 19);
    }
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
    if (!bl1 || bl1->type != BL_MOB)
        return;

    char buf[12];
    TBL_MOB *md = (TBL_MOB *)bl1;

    WBUFW (buf, 0) = 0xb03;
    WBUFW (buf, 2) = 12; // len
    WBUFL (buf, 4) = md->bl.id;
    WBUFL (buf, 8) = md->status.rhw.range;

    clif->send(&buf, sizeof(buf), bl2, target);
}

void send_pc_info(struct block_list* bl1,
                  struct block_list* bl2,
                  enum send_target target)
{
    if (!bl1 || bl1->type != BL_PC)
        return;

    char buf[12];
    TBL_PC *sd = (TBL_PC *)bl1;
    struct SessionExt *data = session_get_bysd(sd);
    if (!data)
        return;

    TBL_PC *tsd = (TBL_PC *)bl2;
    if (tsd)
    {
        struct SessionExt *tdata = session_get_bysd(tsd);
        if (!tdata || tdata->clientVersion < 4)
            return;
    }

    WBUFW (buf, 0) = 0xb0a;
    WBUFW (buf, 2) = 12; // len
    WBUFL (buf, 4) = sd->bl.id;
    if (pc_has_permission(sd, permission_send_gm_flag))
        WBUFL (buf, 8) = sd->group_id;
    else
        WBUFL (buf, 8) = 0;

    clif->send(&buf, sizeof(buf), bl2, target);
}

void send_npc_info(struct block_list* bl1,
                   struct block_list* bl2,
                   enum send_target target)
{
    if (!bl1 || bl1->type != BL_NPC)
        return;

    TBL_PC *tsd = (TBL_PC *)bl2;
    if (tsd)
    {
        struct SessionExt *tdata = session_get_bysd(tsd);
        if (!tdata || tdata->clientVersion < 5)
            return;
    }

    TBL_NPC *const nd = (TBL_NPC*)bl1;

    char buf[12];
    WBUFW (buf, 0) = 0xb0b;
    WBUFW (buf, 2) = 12; // len
    WBUFL (buf, 4) = nd->bl.id;
    WBUFL (buf, 8) = nd->area_size;

    clif->send(&buf, sizeof(buf), bl2, target);
}

void send_advmoving(struct unit_data* ud, bool moving, struct block_list *tbl, enum send_target target)
{
    if (!ud)
        return;

    struct block_list *bl = ud->bl;

    if (ud->walkpath.path_len <= ud->walkpath.path_pos)
        return;
    const bool haveMoves = (ud->walkpath.path_len > ud->walkpath.path_pos);

    int i = 14;
    int start = ud->walkpath.path_pos;
    int len = ud->walkpath.path_len - start;
    if (moving)
    {
        start ++;
        len --;
        if (len <= 0)
            return;
    }
    if (haveMoves)
        i += len;

    char *buf;
    CREATE(buf, char, i);
    WBUFW (buf, 0) = 0xb04;
    WBUFW (buf, 2) = i;
    WBUFL (buf, 4) = bl->id;
    WBUFW (buf, 8) = status->get_speed(bl);
    WBUFW (buf, 10) = bl->x;
    WBUFW (buf, 12) = bl->y;
    if (haveMoves)
        memcpy(buf + 14, ud->walkpath.path + start, len);
    clif->send(buf, i, tbl, target);
    aFree(buf);
}

void send_changemusic_brodcast(const int map, const char *music)
{
    if (!music)
        return;

    struct block_list bl;
    const int sz = strlen (music) + 5;
    char *buf;

    CREATE(buf, char, sz);
    bl.m = map;
    WBUFW (buf, 0) = 0xb05;
    WBUFW (buf, 2) = sz;
    strcpy ((char *)WBUFP (buf, 4), music);
    clif->send (buf, sz, &bl, ALL_SAMEMAP);
    aFree(buf);
}

void send_changenpc_title (TBL_PC *sd, const int npcId, const char *name)
{
    if (!sd || !name)
        return;

    const int fd = sd->fd;
    const int len = strlen (name);
    const int sz = len + 5 + 4 + 2;
    WFIFOHEAD (fd, sz);
    WFIFOW (fd, 0) = 0xb06;
    WFIFOW (fd, 2) = sz;
    WFIFOL (fd, 4) = npcId;
    WFIFOW (fd, 8) = len;
    strcpy ((char*)WFIFOP (fd, 10), name);
    WFIFOSET (fd, sz);
}

void send_join_ack(int fd, const char *const name, int flag)
{
    if (!name)
        return;

    WFIFOHEAD (fd, 27);
    WFIFOW (fd, 0) = 0xb08;
    safestrncpy ((char*)WFIFOP (fd, 2), name, 24);
    WFIFOB (fd, 26) = flag;
    WFIFOSET (fd, 27);
}

void send_slave_say(TBL_PC *sd,
                    struct block_list *bl,
                    const char *const name,
                    const char *const message)
{
    const int len = 24 + 7 + strlen(message);
    char *buf = NULL;
    CREATE(buf, char, len);

    snprintf(buf, len, "%s's %s : %s", sd->status.name, name, message);
    buf[len - 1] = 0;
    clif->GlobalMessage(bl, buf);
    aFree(buf);
}

void send_online_list(int fd, const char *buf, unsigned size)
{
    if (!buf)
        return;
    const unsigned int len = size + 4 + 1;
    WFIFOHEAD (fd, len);
    WFIFOW (fd, 0) = 0xb10;
    WFIFOW (fd, 2) = len;
    memcpy (WFIFOP (fd, 4), buf, size);
    WFIFOB (fd, size + 4) = 0;
    WFIFOSET (fd, len);
}

void send_client_command(TBL_PC *sd, const char *const command)
{
    struct SessionExt *data = session_get_bysd(sd);
    if (!data || data->clientVersion < 8)
        return;

    const unsigned int len = strlen(command);
    const int fd = sd->fd;
    WFIFOHEAD (fd, len);
    WFIFOW (fd, 0) = 0xb16;
    WFIFOW (fd, 2) = len + 4;
    memcpy (WFIFOP (fd, 4), command, len);
    WFIFOSET (fd, len + 4);
}

void send_changelook2(struct map_session_data* sd, struct block_list *bl, int id, int type, int val, int val2,
                      struct item_data *data, int n, enum send_target target)
{
    //ShowWarning("equip: for type %d = %d\n", type, val);
    unsigned char buf[32];
    int i;

    WBUFW(buf, 0) = 0x1d7;
    WBUFL(buf, 2) = id;
    WBUFB(buf, 6) = type;
    WBUFW(buf, 7) = val;
    WBUFW(buf, 9) = val2;
    clif->send(buf, 11, bl, target);
    WBUFW(buf, 0) = 0xb17;
    if (data)
    {
        //ShowWarning("equip: for type %d\n", type);
        for (i = 0; i < data->slot; i++ )
        {
            struct item_data *data;
            if (!sd->status.inventory[n].card[i])
                continue;
            if ((data = itemdb->exists(sd->status.inventory[n].card[i])) != NULL)
            {
                //ShowWarning("card %d\n", data->nameid);
                WBUFW(buf, 11 + i * 2) = data->nameid;
            }
        }
        for (i = data->slot; i < MAX_SLOTS; i ++)
            WBUFW(buf, 11 + i * 2) = 0;
    }
    else
    {
        //ShowWarning("unequip: for type %d\n", type);
        WBUFW(buf, 11) = 0;
        WBUFW(buf, 13) = 0;
        WBUFW(buf, 15) = 0;
        WBUFW(buf, 17) = 0;
    }
    clif->send(buf, 19, bl, target);
}
