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
    hookStop();
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
        WFIFOL(fd, i * info_len + 8) = sd->quest_log[i].quest_id;
        WFIFOB(fd, i * info_len + 12) = sd->quest_log[i].count[0]; // was state
        WFIFOL(fd, i * info_len + 13) = sd->quest_log[i].time - qi->time;
        WFIFOL(fd, i * info_len + 17) = sd->quest_log[i].time;
        WFIFOW(fd, i * info_len + 21) = 0;
    }

    WFIFOSET(fd, len);
}

void eclif_quest_add(struct map_session_data *sd, struct quest *qd)
{
    hookStop();
    int fd = sd->fd;
    struct quest_db *qi = quest->db(qd->quest_id);

    WFIFOHEAD(fd, packet_len(0x2b3));
    WFIFOW(fd, 0) = 0x2b3;
    WFIFOL(fd, 2) = qd->quest_id;
    WFIFOB(fd, 6) = qd->count[0]; // was state;
    WFIFOB(fd, 7) = qd->time - qi->time;
    WFIFOL(fd, 11) = qd->time;
    WFIFOW(fd, 15) = 0;

    WFIFOSET(fd, 107);
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
        hookStop();
        int fd = *fdPtr;
        struct map_session_data* sd = (struct map_session_data*)session[fd]->session_data;
        const char *tr = lang_pctrans(((TBL_NPC*)bl)->name, sd);
        const int len = 8 + strlen(tr) + 1;
        // if no recipient specified just update nearby clients
        if (fd == 0)
        {
            char *buf;
            CREATE(buf, char, len);
            WBUFW(buf, 0) = 0xB01;
            WBUFW(buf, 2) = len;
            WBUFL(buf, 4) = bl->id;
            memcpy(WBUFP(buf, 8), tr, len);
            clif->send(buf, len, bl, AREA);
            aFree(buf);
        }
        else
        {
            WFIFOHEAD(fd, len);
            WFIFOW(fd, 0) = 0xB01;
            WFIFOW(fd, 2) = len;
            WFIFOL(fd, 4) = bl->id;
            memcpy(WFIFOP(fd, 8), tr, len);
            WFIFOSET(fd, len);
        }
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
    const int id = sd->bl.id;
    const int fd = sd2->fd;

    struct item_data *item;
    short equip;
    struct MapdExt *data = mapd_get(sd->bl.m);
    if (data->invisible)
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
    struct MapdExt *data = mapd_get(sd->bl.m);
    int mask = data ? data->mask : 1;
    send_mapmask(sd->fd, mask);
}

void eclif_changemap_post(struct map_session_data *sd, short *m, int *x, int *y)
{
    struct MapdExt *data = mapd_get(sd->bl.m);
    int mask = data ? data->mask : 1;
    send_mapmask(sd->fd, mask);
}

void eclif_handle_invisible_map(struct block_list *bl, enum send_target target)
{
    if (!bl || bl->type != BL_PC)
        return;
    struct MapdExt *data = mapd_get(bl->m);
    if (data->invisible)
        hookStop();
}

void eclif_sendlook(struct block_list *bl, int *id, int *type, int *val, int *val2, enum send_target *target)
{
    if (*target == SELF)
        return;
    eclif_handle_invisible_map(bl, *target);
}

bool eclif_send(const void* buf, int *len, struct block_list* bl, enum send_target *type)
{
    if (*type == SELF)
        return;
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
        if (packet == 0xb02 || packet == 0xb03)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion < 3)
            {   // not sending new packets to old clients
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

    if (bl->type == BL_MOB && tsd)
        send_mob_info(bl, tsd ? &tsd->bl : bl, *target);
}
