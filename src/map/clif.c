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
#include "../../../common/cbasetypes.h"
#include "../../../map/npc.h"
#include "../../../map/pc.h"
#include "../../../map/quest.h"

#include "map/clif.h"
#include "map/lang.h"
#include "map/send.h"
#include "map/data/mapd.h"
#include "map/data/session.h"
#include "map/struct/mapdext.h"
#include "map/struct/sessionext.h"

void eclif_quest_send_list(struct map_session_data *sd)
{
    if (!sd)
    {
        hookStop();
        return;
    }

    int fd = sd->fd;
    int i;
    int info_len = 15;
    int len = sd->avail_quests * info_len + 8;
    WFIFOHEAD(fd,len);
    WFIFOW(fd, 0) = 0x97a;
    WFIFOW(fd, 2) = len;
    WFIFOL(fd, 4) = sd->avail_quests;

    for (i = 0; i < sd->avail_quests; i++ )
    {
        struct quest_db *qi = quest->db(sd->quest_log[i].quest_id);
        if (!qi)
            continue;
        WFIFOL(fd, i * info_len + 8) = sd->quest_log[i].quest_id;
        WFIFOB(fd, i * info_len + 12) = sd->quest_log[i].count[0]; // was state
        WFIFOL(fd, i * info_len + 13) = sd->quest_log[i].time - qi->time;
        WFIFOL(fd, i * info_len + 17) = sd->quest_log[i].time;
        WFIFOW(fd, i * info_len + 21) = 0;
    }

    WFIFOSET(fd, len);
    hookStop();
}

void eclif_quest_add(struct map_session_data *sd, struct quest *qd)
{
    if (!sd)
    {
        hookStop();
        return;
    }
    int fd = sd->fd;
    struct quest_db *qi = quest->db(qd->quest_id);

    if (!qi)
    {
        hookStop();
        return;
    }

    WFIFOHEAD(fd, packet_len(0x2b3));
    WFIFOW(fd, 0) = 0x2b3;
    WFIFOL(fd, 2) = qd->quest_id;
    WFIFOB(fd, 6) = qd->count[0]; // was state;
    WFIFOB(fd, 7) = qd->time - qi->time;
    WFIFOL(fd, 11) = qd->time;
    WFIFOW(fd, 15) = 0;

    WFIFOSET(fd, 107);
    hookStop();
}

void eclif_charnameack(int *fdPtr, struct block_list *bl)
{
    if (!bl)
    {
        hookStop();
        return;
    }
    if (bl->type == BL_NPC)
    {
        int fd = *fdPtr;
        struct map_session_data* sd = (struct map_session_data*)session[fd]->session_data;
        if (!sd)
        {
            hookStop();
            return;
        }
        const char *tr = lang_pctrans(((TBL_NPC*)bl)->name, sd);
        const int trLen = strlen(tr);
        const int len = 8 + trLen;
        // if no recipient specified just update nearby clients
        if (fd == 0)
        {
            char *buf;
            CREATE(buf, char, len);
            WBUFW(buf, 0) = 0xB01;
            WBUFW(buf, 2) = len;
            WBUFL(buf, 4) = bl->id;
            memcpy(WBUFP(buf, 8), tr, trLen);
            clif->send(buf, len, bl, AREA);
            aFree(buf);
        }
        else
        {
            WFIFOHEAD(fd, len);
            WFIFOW(fd, 0) = 0xB01;
            WFIFOW(fd, 2) = len;
            WFIFOL(fd, 4) = bl->id;
            memcpy(WFIFOP(fd, 8), tr, trLen);
            WFIFOSET(fd, len);
        }
        hookStop();
    }
}

#define equipPos(index, field) \
    equip = sd->equip_index[index]; \
    if (equip >= 0) \
    { \
        item = sd->inventory_data[equip]; \
        if (item && item->look) \
            send_changelook(fd, id, field, item->look); \
    }

static void eclif_send_additional_slots(struct map_session_data* sd, struct map_session_data* sd2)
{
    if (!sd || !sd2)
        return;

    const int id = sd->bl.id;
    const int fd = sd2->fd;

    struct item_data *item;
    short equip;
    struct MapdExt *data = mapd_get(sd->bl.m);
    if (!data || data->invisible)
        return;

    equipPos(EQI_HEAD_LOW, LOOK_HEAD_BOTTOM);
    equipPos(EQI_HEAD_TOP, LOOK_HEAD_TOP);
    equipPos(EQI_HEAD_MID, LOOK_HEAD_MID);
    equipPos(EQI_GARMENT, LOOK_ROBE);
    equipPos(EQI_SHOES, LOOK_SHOES);
    equipPos(EQI_COSTUME_TOP, 13);
    equipPos(EQI_COSTUME_MID, 14);
    equipPos(EQI_COSTUME_LOW, 15);
    equipPos(EQI_COSTUME_GARMENT, 16);
    equipPos(EQI_ARMOR, 17);
    //skipping SHADOW slots
}

void eclif_getareachar_unit_post(struct map_session_data* sd, struct block_list *bl)
{
    if (!bl)
        return;
    if (bl->type == BL_PC)
    {
        eclif_send_additional_slots(sd, (struct map_session_data *)bl);
        eclif_send_additional_slots((struct map_session_data *)bl, sd);
    }
}

void eclif_authok_post(struct map_session_data *sd)
{
    if (!sd)
        return;

    eclif_send_additional_slots(sd, sd);
    send_pc_info(&sd->bl, &sd->bl, SELF);
    struct MapdExt *data = mapd_get(sd->bl.m);
    int mask = data ? data->mask : 1;
    send_mapmask(sd->fd, mask);
}

void eclif_changemap_post(struct map_session_data *sd, short *m,
                          int *x __attribute__ ((unused)), int *y __attribute__ ((unused)))
{
    if (!sd)
        return;
    struct MapdExt *data = mapd_get(*m);
    int mask = data ? data->mask : 1;
    send_mapmask(sd->fd, mask);
}

void eclif_handle_invisible_map(struct block_list *bl, enum send_target target __attribute__ ((unused)))
{
    if (!bl || bl->type != BL_PC)
        return;
    struct MapdExt *data = mapd_get(bl->m);
    if (data && data->invisible)
        hookStop();
}

void eclif_sendlook(struct block_list *bl,
                    int *id __attribute__ ((unused)),
                    int *type __attribute__ ((unused)),
                    int *val __attribute__ ((unused)),
                    int *val2 __attribute__ ((unused)),
                    enum send_target *target)
{
    if (*target == SELF)
        return;
    eclif_handle_invisible_map(bl, *target);
}

bool eclif_send(const void* buf __attribute__ ((unused)),
                int *len __attribute__ ((unused)),
                struct block_list* bl,
                enum send_target *type)
{
    if (*type == SELF)
        return true;
    eclif_handle_invisible_map(bl, *type);
    return true;
}

void eclif_set_unit_idle(struct block_list* bl, struct map_session_data *tsd, enum send_target *target)
{
    if (tsd && bl && bl->id == tsd->bl.id && *target == SELF)
        return;

    eclif_handle_invisible_map(bl, *target);
}

int eclif_send_actual(int *fd, void *buf, int *len)
{
    if (*len >= 2)
    {
        const int packet = RBUFW (buf, 0);
        if (packet >= 0xb02 && packet <= 0xb10)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion < 3)
            {   // not sending new packets to old clients
//                ShowWarning("skip packet %d\n", packet);
                hookStop();
                return 0;
            }
        }
        if (packet >= 0xb03 && packet <= 0xb0a)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion < 4)
            {   // not sending new packets to old clients
//                ShowWarning("skip packet %d\n", packet);
                hookStop();
                return 0;
            }
        }
        if (packet == 0xb0b)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion < 5)
            {   // not sending new packets to old clients
//                ShowWarning("skip packet %d\n", packet);
                hookStop();
                return 0;
            }
        }
    }
    return 0;
}

void eclif_set_unit_idle_post(struct block_list* bl, struct map_session_data *tsd,
                              enum send_target *target)
{
    if (!bl || !tsd)
        return;

    if (bl->type == BL_MOB)
        send_mob_info(bl, &tsd->bl, *target);
    else if (bl->type == BL_PC)
        send_pc_info(bl, &tsd->bl, *target);
    else if (bl->type == BL_NPC)
        send_npc_info(bl, &tsd->bl, *target);
}

void eclif_set_unit_walking(struct block_list* bl, struct map_session_data *tsd,
                            struct unit_data* ud, enum send_target *target)
{
    send_advmoving(ud, tsd ? &tsd->bl : bl, *target);
}

void eclif_move(struct unit_data *ud)
{
    send_advmoving(ud, ud->bl,  AREA_WOS);
}
