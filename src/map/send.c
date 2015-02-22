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
#include "../../../map/clif.h"
#include "../../../map/mob.h"
#include "../../../map/npc.h"
#include "../../../map/pc.h"
#include "../../../map/pet.h"
#include "../../../map/unit.h"

#include "map/send.h"
#include "map/permission.h"
#include "map/data/session.h"
#include "map/struct/sessionext.h"

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

void send_pc_info(struct block_list* bl1,
                  struct block_list* bl2,
                  enum send_target target)
{
    char buf[12];

    if (bl1->type != BL_PC)
        return;

    struct map_session_data *sd = (struct map_session_data *)bl1;
    struct SessionExt *data = session_get_bysd(sd);
    if (!data)
        return;

    struct map_session_data *tsd = (struct map_session_data *)bl2;
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

    struct map_session_data *tsd = (struct map_session_data *)bl2;
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

void send_advmoving(struct unit_data* ud, struct block_list *tbl, enum send_target target)
{
    if (!ud)
        return;

    struct block_list *bl = ud->bl;

    if (ud->walkpath.path_len <= ud->walkpath.path_pos)
        return;
    const bool haveMoves = (ud->walkpath.path_len > ud->walkpath.path_pos);

    int i = 14;
    const int len = ud->walkpath.path_len - ud->walkpath.path_pos;
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
        memcpy(buf + 14, ud->walkpath.path + ud->walkpath.path_pos, len);
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

void send_changenpc_title (struct map_session_data *sd, const int npcId, const char *name)
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
    WFIFOHEAD (fd, 27);
    WFIFOW (fd, 0) = 0xb08;
    safestrncpy ((char*)WFIFOP (fd, 2), name, 24);
    WFIFOB (fd, 26) = flag;
    WFIFOSET (fd, 27);
}

void send_pet_say(struct map_session_data *sd, const char *const message)
{
    if (!sd || !sd->pd)
        return;

    const char *const name = sd->pd->pet.name;
    const int len = 24 + 7 + strlen(message);
    char *buf = NULL;
    CREATE(buf, char, len);

    snprintf(buf, len, "%s's %s : %s", sd->status.name, name, message);
    buf[len - 1] = 0;
    clif->GlobalMessage(&sd->pd->bl, buf);
    aFree(buf);
}

void send_pet_emote(struct map_session_data *sd, const int emote)
{
    if (!sd || !sd->pd)
        return;

    clif->emotion(&sd->pd->bl, emote);
}

void send_online_list(int fd, const char *buf, unsigned size)
{
    const unsigned int len = size + 4 + 1;
    WFIFOHEAD (fd, len);
    WFIFOW (fd, 0) = 0xb10;
    WFIFOW (fd, 2) = len;
    memcpy (WFIFOP (fd, 4), buf, size);
    WFIFOB (fd, size + 4) = 0;
    WFIFOSET (fd, len);
}
