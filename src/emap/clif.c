// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/cbasetypes.h"
#include "common/random.h"
#include "common/timer.h"
#include "map/guild.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/quest.h"

#include "plugins/HPMHooking.h"

#include "emap/clif.h"
#include "emap/lang.h"
#include "emap/map.h"
#include "emap/packets_struct.h"
#include "emap/send.h"
#include "emap/data/itemd.h"
#include "emap/data/mapd.h"
#include "emap/data/session.h"
#include "emap/struct/itemdext.h"
#include "emap/struct/mapdext.h"
#include "emap/struct/sessionext.h"

extern bool isInit;
extern char global_npc_str[1001];

static inline void RBUFPOS(const uint8 *p,
                           unsigned short pos,
                           short *x,
                           short *y,
                           unsigned char *dir)
{
    p += pos;

    if (x)
    {
        x[0] = ((p[0] & 0xff) << 2) | (p[1] >> 6);
    }

    if (y)
    {
        y[0] = ((p[1] & 0x3f) << 4) | (p[2] >> 4);
    }

    if (dir)
    {
        dir[0] = (p[2] & 0x0f);
    }
}

static inline void RFIFOPOS(int fd,
                            unsigned short pos,
                            short *x,
                            short *y,
                            unsigned char *dir)
{
    RBUFPOS(RFIFOP(fd,pos), 0, x, y, dir);
}

void eclif_quest_send_list_pre(TBL_PC **sdPtr)
{
    TBL_PC *sd = *sdPtr;

    if (!sd)
    {
        hookStop();
        return;
    }

    struct SessionExt *data = session_get_bysd(sd);
    if (!data)
        return;
    if (data->clientVersion < 20)
    {
        int fd = sd->fd;
        int i;
        int info_len = 15;
        int len = sd->avail_quests * info_len + 8;
        WFIFOHEAD(fd, len);
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
    }
    else
    {  // data->clientVersion >= 20
        int fd = sd->fd;
        int info_len = 4 + 1 + 3 * 4 + 4;
        int len = sd->avail_quests * info_len + 8;
        WFIFOHEAD(fd, len);
        WFIFOW(fd, 0) = 0xb23;
        WFIFOW(fd, 2) = len;
        WFIFOL(fd, 4) = sd->avail_quests;
        for (int i = 0; i < sd->avail_quests; i++ )
        {
            struct quest_db *qi = quest->db(sd->quest_log[i].quest_id);
            if (!qi)
                continue;
            WFIFOL(fd, i * info_len + 8) = sd->quest_log[i].quest_id;
            WFIFOB(fd, i * info_len + 12) = sd->quest_log[i].state;
            WFIFOL(fd, i * info_len + 13) = sd->quest_log[i].count[0];
            WFIFOL(fd, i * info_len + 17) = sd->quest_log[i].count[1];
            WFIFOL(fd, i * info_len + 21) = sd->quest_log[i].count[2];
            WFIFOL(fd, i * info_len + 25) = sd->quest_log[i].time;
        }
        WFIFOSET(fd, len);
    }

    hookStop();
}

void eclif_quest_add_pre(TBL_PC **sdPtr,
                         struct quest **qdPtr)
{
    eclif_quest_add(*sdPtr, *qdPtr);
}

void eclif_quest_add(TBL_PC *sd,
                     struct quest *qd)
{
    // for new client need send here: id, 3 counts, time
    if (!sd || !qd)
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

    struct SessionExt *data = session_get_bysd(sd);
    if (!data)
        return;
    if (data->clientVersion < 20)
    {
        WFIFOHEAD(fd, 107);
        WFIFOW(fd, 0) = 0x2b3;
        WFIFOL(fd, 2) = qd->quest_id;
        WFIFOB(fd, 6) = qd->count[0]; // was state;
        WFIFOB(fd, 7) = qd->time - qi->time;
        WFIFOL(fd, 11) = qd->time;
        WFIFOW(fd, 15) = 0;
        WFIFOSET(fd, 107);
    }
    else
    {  // data->clientVersion >= 20
        WFIFOHEAD(fd, 23);
        WFIFOW(fd, 0) = 0xb24;
        WFIFOL(fd, 2) = qd->quest_id;
        WFIFOB(fd, 6) = qd->state;
        WFIFOL(fd, 7) = qd->count[0];
        WFIFOL(fd, 11) = qd->count[1];
        WFIFOL(fd, 15) = qd->count[2];
        WFIFOL(fd, 19) = qd->time;
        WFIFOSET(fd, 23);
    }
    hookStop();
}

void eclif_charnameack_pre(int *fdPtr,
                           struct block_list **blPtr)
{
    struct block_list *bl = *blPtr;
    if (!bl)
    {
        hookStop();
        return;
    }
    if (bl->type == BL_NPC)
    {
        int fd = *fdPtr;
        TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
        if (!sd)
        {
            hookStop();
            return;
        }
        const char *tr = lang_pctrans(((TBL_NPC*)bl)->name, sd);
        const int trLen = (int)strlen(tr);
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
    else if (bl->type == BL_MOB)
    {
        struct mob_data *md = (struct mob_data *)bl;
        if (!md)
        {
            hookStop();
            return;
        }
        if (md->guardian_data && md->guardian_data->g)
            return; // allow default code to work
        int fd = *fdPtr;
        TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
        if (!sd)
        {
            hookStop();
            return;
        }

        char tmpBuf[25];
        char *ptr = tmpBuf;
        int f;
        memcpy(tmpBuf, md->name, 24);
        tmpBuf[24] = 0;
        for (f = 23; f > 1; f --)
        {
            if (tmpBuf[f] == ' ')
                tmpBuf[f] = 0;
            else
                break;
        }
        for (f = 0; f < 24; f ++)
        {
            if (*ptr == ' ')
                ptr ++;
            else
                break;
        }
        const char *tr = lang_pctrans(ptr, sd);
        const int trLen = (int)strlen(tr);
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
            send_changelook(sd, sd2, fd, id, field, item->look, 0, item, equip); \
    }

#define equipPosId(index, field) \
    equip = sd->equip_index[index]; \
    if (equip >= 0) \
    { \
        item = sd->inventory_data[equip]; \
        if (item && item->nameid) \
            send_changelook(sd, sd2, fd, id, field, item->nameid, 0, item, equip); \
    }

static void eclif_send_additional_slots(TBL_PC* sd, TBL_PC* sd2)
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

    equipPosId(EQI_HAND_R, LOOK_WEAPON);
    equipPosId(EQI_HAND_L, LOOK_SHIELD);
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
    equipPos(EQI_ACC_R, 18);
    equipPos(EQI_ACC_L, 19);
    //skipping SHADOW slots
}

#undef equipPos
#undef equipPosId

#define equipPos2(index, field) \
    equip = sd->equip_index[index]; \
    if (equip >= 0) \
    { \
        item = sd->inventory_data[equip]; \
        if (item && item->look) \
        { \
            send_changelook2(sd, bl, bl->id, field, item->look, 0, item, equip, AREA); \
        } \
    }

#define equipPos2Id(index, field) \
    equip = sd->equip_index[index]; \
    if (equip >= 0) \
    { \
        item = sd->inventory_data[equip]; \
        if (item && item->nameid) \
        { \
            send_changelook2(sd, bl, bl->id, field, item->nameid, 0, item, equip, AREA); \
        } \
    }

static void eclif_send_additional_slots2(struct block_list *bl)
{
    if (!bl)
        return;

    TBL_PC* sd = (TBL_PC*)bl;
    struct item_data *item;
    short equip;
    struct MapdExt *data = mapd_get(sd->bl.m);
    if (!data || data->invisible)
        return;

    equipPos2Id(EQI_HAND_R, LOOK_WEAPON);
    equipPos2Id(EQI_HAND_L, LOOK_SHIELD);
    equipPos2(EQI_HEAD_LOW, LOOK_HEAD_BOTTOM);
    equipPos2(EQI_HEAD_TOP, LOOK_HEAD_TOP);
    equipPos2(EQI_HEAD_MID, LOOK_HEAD_MID);
    equipPos2(EQI_GARMENT, LOOK_ROBE);
    equipPos2(EQI_SHOES, LOOK_SHOES);
    equipPos2(EQI_COSTUME_TOP, 13);
    equipPos2(EQI_COSTUME_MID, 14);
    equipPos2(EQI_COSTUME_LOW, 15);
    equipPos2(EQI_COSTUME_GARMENT, 16);
    equipPos2(EQI_ARMOR, 17);
    equipPos2(EQI_ACC_R, 18);
    equipPos2(EQI_ACC_L, 19);
    //skipping SHADOW slots
}

#undef equipPos2
#undef equipPos2Id

void eclif_getareachar_unit_post(TBL_PC *sd,
                                 struct block_list *bl)
{
    if (!bl || !sd)
        return;
    if (bl->type == BL_PC)
    {
        eclif_send_additional_slots((TBL_PC *)bl, sd);
        send_pc_info(bl, &sd->bl, SELF);
    }
}

bool eclif_spawn_post(bool retVal,
                      struct block_list *bl)
{
    if (!bl)
        return retVal;
    if (retVal == true && bl->type == BL_PC)
    {
        send_pc_info(bl, bl, AREA);
        eclif_send_additional_slots2(bl);
    }
    return retVal;
}

void eclif_authok_post(TBL_PC *sd)
{
    if (!sd)
        return;

    eclif_send_additional_slots(sd, sd);
    send_pc_info(&sd->bl, &sd->bl, SELF);
    struct MapdExt *data = mapd_get(sd->bl.m);
    int mask = data ? data->mask : 1;
    send_mapmask(sd->fd, mask);
}

void eclif_changemap_post(TBL_PC *sd,
                          short m,
                          int x __attribute__ ((unused)),
                          int y __attribute__ ((unused)))
{
    if (!sd)
        return;
    struct MapdExt *data = mapd_get(m);
    int mask = data ? data->mask : 1;
    send_mapmask(sd->fd, mask);
}

void eclif_handle_invisible_map(struct block_list *bl,
                                enum send_target target __attribute__ ((unused)))
{
    if (!bl || bl->type != BL_PC)
        return;
    struct MapdExt *data = mapd_get(bl->m);
    if (data && data->invisible)
        hookStop();
}

void eclif_sendlook_pre(struct block_list **blPtr,
                        int *id __attribute__ ((unused)),
                        int *type __attribute__ ((unused)),
                        int *val __attribute__ ((unused)),
                        int *val2 __attribute__ ((unused)),
                        enum send_target *target)
{
    struct block_list *bl = *blPtr;
    if (*target == SELF)
        return;
    eclif_handle_invisible_map(bl, *target);
}

bool eclif_send_pre(const void **bufPtr,
                    int *len __attribute__ ((unused)),
                    struct block_list **blPtr,
                    enum send_target *type)
{
    struct block_list *bl = *blPtr;
    const void *buf = *bufPtr;
    if (*type == SELF)
    {
        if (*len >= 2)
        {
            const int packet = RBUFW (buf, 0);
            if (packet == 0x9dd || packet == 0x9dc || packet == 0x9db || packet == 0x8c8)
            {
                struct map_session_data *sd = BL_CAST(BL_PC, bl);
                struct SessionExt *data = session_get_bysd(sd);
                if (!data)
                    return true;
                if (data->clientVersion < 16)
                {   // not sending new packet to old clients
                    hookStop();
                    return true;
                }
            }
            if (packet == 0x915 || packet == 0x90f || packet == 0x914 || packet == 0x2e1)
            {
                struct map_session_data *sd = BL_CAST(BL_PC, bl);
                struct SessionExt *data = session_get_bysd(sd);
                if (!data)
                    return true;
                if (data->clientVersion >= 16)
                {   // not sending old packet to new clients
                    hookStop();
                    return true;
                }
            }
            if (packet == 0x9cb)
            {
                struct map_session_data *sd = BL_CAST(BL_PC, bl);
                struct SessionExt *data = session_get_bysd(sd);
                if (!data)
                    return true;
                if (data->clientVersion < 19)
                {   // not sending new packet to old clients
                    hookStop();
                    return true;
                }
            }
        }
        return true;
    }
    eclif_handle_invisible_map(bl, *type);
    return true;
}

void eclif_set_unit_idle_pre(struct block_list **blPtr,
                             TBL_PC **tsdPtr,
                             enum send_target *target)
{
    struct block_list *bl = *blPtr;
    TBL_PC *tsd = *tsdPtr;
    if (tsd && bl && bl->id == tsd->bl.id && *target == SELF)
        return;

    eclif_handle_invisible_map(bl, *target);
}

int eclif_send_actual_pre(int *fd,
                          void **bufPtr,
                          int *len)
{
    void *buf = *bufPtr;
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
        if (packet == 0x1d7)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion >= 9)
            {   // not sending old packets to new clients
                hookStop();
                return 0;
            }
        }
        if (packet == 0xb17)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion < 9)
            {   // not sending new packets to old clients
                hookStop();
                return 0;
            }
        }
        if (packet == 0x84b)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion >= 10)
            {   // not sending old packets to new clients
                hookStop();
                return 0;
            }
        }
        if (packet == 0xb19)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion < 10)
            {   // not sending new packets to old clients
                hookStop();
                return 0;
            }
        }
        if (packet == 0x2dd)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion >= 12)
            {   // not sending old packets to new clients
                hookStop();
                return 0;
            }
        }
        if (packet == 0xb1a)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion < 12)
            {   // not sending new packets to old clients
                hookStop();
                return 0;
            }
        }
        if (packet == 0xb1b)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion < 14)
            {   // not sending new packets to old clients
                hookStop();
                return 0;
            }
        }
        if (packet == 0x9dd || packet == 0x9dc || packet == 0x9db || packet == 0x8c8)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion < 16)
            {   // not sending new packets to old clients
                hookStop();
                return 0;
            }
        }
        if (packet == 0x915 || packet == 0x90f || packet == 0x914 || packet == 0x2e1)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion >= 16)
            {   // not sending old packets to new clients
                hookStop();
                return 0;
            }
        }
        if (packet == 0xb1e)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion < 18)
            {   // not sending new packets to old clients
//                ShowWarning("skip packet %d\n", packet);
                hookStop();
                return 0;
            }
        }
        if (packet == 0x7fb)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion >= 18)
            {   // not sending old packets to new clients
                hookStop();
                return 0;
            }
        }
        if (packet == 0x9cb)
        {
            struct SessionExt *data = session_get(*fd);
            if (!data)
                return 0;
            if (data->clientVersion < 19)
            {   // not sending new packets to old clients
                hookStop();
                return 0;
            }
        }
    }
    return 0;
}

uint16 GetWord(uint32 val, int idx)
{
    switch( idx )
    {
        case 0: return (uint16)( (val & 0x0000FFFF)         );
        case 1: return (uint16)( (val & 0xFFFF0000) >> 0x10 );
        default:
#if defined(DEBUG)
            ShowDebug("GetWord: invalid index (idx=%d)\n", idx);
#endif
            return 0;
    }
}

//To make the assignation of the level based on limits clearer/easier. [Skotlex]
static int clif_setlevel_sub(int lv)
{
    if (lv < battle->bc->max_lv)
    {
        ;
    }
    else if (lv < battle->bc->aura_lv)
    {
        lv = battle->bc->max_lv - 1;
    }
    else
    {
        lv = battle->bc->max_lv;
    }

    return lv;
}

static int clif_setlevel(struct block_list* bl)
{
    int lv = status->get_lv(bl);
    nullpo_retr(0, bl);
    if (battle->bc->client_limit_unit_lv&bl->type)
        return clif_setlevel_sub(lv);
    if (bl->type == BL_NPC || bl->type == BL_PET)
        return 0;
    return lv;
}

//To identify disguised characters.
static inline bool disguised(struct block_list* bl)
{
    return (bool)(bl &&
        bl->type == BL_PC &&
        ((TBL_PC*)bl)->disguise != -1);
}

static inline void WBUFPOS(uint8* p, unsigned short pos, short x, short y, unsigned char dir)
{
    p += pos;
    p[0] = (uint8)(x >> 2);
    p[1] = (uint8)((x << 6) | ((y >> 4) & 0x3f));
    p[2] = (uint8)((y << 4) | (dir & 0xf));
}

// client-side: x0+=sx0*0.0625-0.5 and y0+=sy0*0.0625-0.5
static inline void WBUFPOS2(uint8* p, unsigned short pos, short x0, short y0, short x1, short y1, unsigned char sx0, unsigned char sy0)
{
    p += pos;
    p[0] = (uint8)(x0>>2);
    p[1] = (uint8)((x0<<6) | ((y0>>4)&0x3f));
    p[2] = (uint8)((y0<<4) | ((x1>>6)&0x0f));
    p[3] = (uint8)((x1<<2) | ((y1>>8)&0x03));
    p[4] = (uint8)y1;
    p[5] = (uint8)((sx0<<4) | (sy0&0x0f));
}

static inline unsigned char clif_bl_type_old(struct block_list *bl)
{
    nullpo_retr(0x1, bl);
    switch (bl->type)
    {
        case BL_PC:    return (disguised(bl) && !pc->db_checkid(status->get_viewdata(bl)->class))? 0x1:0x0; //PC_TYPE
        case BL_ITEM:  return 0x2; //ITEM_TYPE
        case BL_SKILL: return 0x3; //SKILL_TYPE
        case BL_CHAT:  return 0x4; //UNKNOWN_TYPE
        case BL_MOB:   return pc->db_checkid(status->get_viewdata(bl)->class)?0x0:0x5; //NPC_MOB_TYPE
        case BL_NPC:   return pc->db_checkid(status->get_viewdata(bl)->class)?0x0:0x6; //NPC_EVT_TYPE
        case BL_PET:   return pc->db_checkid(status->get_viewdata(bl)->class)?0x0:0x7; //NPC_PET_TYPE
        case BL_HOM:   return 0x8; //NPC_HOM_TYPE
        case BL_MER:   return 0x9; //NPC_MERSOL_TYPE
        case BL_ELEM:  return 0xa; //NPC_ELEMENTAL_TYPE
        default:       return 0x1; //NPC_TYPE
    }
}

//Modifies the type of damage according to status changes [Skotlex]
//Aegis data specifies that: 4 endure against single hit sources, 9 against multi-hit.
static inline int clif_calc_delay(int type, int div, int damage, int delay)
{
    return (delay == 0 && damage > 0) ? (div > 1 ? 9 : 4) : type;
}

// this function must be used only by clients version < 16
void eclif_set_unit_idle_old(struct block_list* bl,
                             struct map_session_data *tsd,
                             enum send_target target)
{
    struct map_session_data* sd;
    struct status_change* sc = status->get_sc(bl);
    struct view_data* vd = status->get_viewdata(bl);
    struct packet_idle_unit_old p;
    int g_id = status->get_guild_id(bl);

    nullpo_retv(bl);

    sd = BL_CAST(BL_PC, bl);

    p.PacketType = 0x915;
    p.PacketLength = sizeof(p);
    p.objecttype = clif_bl_type_old(bl);
//    p.AID = bl->id;
//    p.GID = (sd) ? sd->status.char_id : 0;    // CCODE
    p.GID = bl->id;
    p.speed = status->get_speed(bl);
    p.bodyState = (sc) ? sc->opt1 : 0;
    p.healthState = (sc) ? sc->opt2 : 0;
    p.effectState = (sc) ? sc->option : bl->type == BL_NPC ? ((TBL_NPC*)bl)->option : 0;
    p.job = vd->class;
    p.head = vd->hair_style;
    p.weapon = vd->weapon;
    p.accessory = vd->head_bottom;
    p.accessory2 = vd->head_top;
    p.accessory3 = vd->head_mid;
    if (bl->type == BL_NPC && vd->class == FLAG_CLASS)
    {   //The hell, why flags work like this?
        p.accessory = status->get_emblem_id(bl);
        p.accessory2 = GetWord(g_id, 1);
        p.accessory3 = GetWord(g_id, 0);
    }
    p.headpalette = vd->hair_color;
    p.bodypalette = vd->cloth_color;
    p.headDir = (sd)? sd->head_dir : 0;
    p.robe = vd->robe;
    p.GUID = g_id;
    p.GEmblemVer = status->get_emblem_id(bl);
    p.honor = (sd) ? sd->status.manner : 0;
    p.virtue = (sc) ? sc->opt3 : 0;
    p.isPKModeON = (sd && sd->status.karma) ? 1 : 0;
    p.sex = vd->sex;
    WBUFPOS(&p.PosDir[0],0,bl->x,bl->y,unit->getdir(bl));
    p.xSize = p.ySize = (sd) ? 5 : 0;
    p.state = vd->dead_sit;
    p.clevel = clif_setlevel(bl);
    p.font = (sd) ? sd->status.font : 0;
    if (battle->bc->show_monster_hp_bar && bl->type == BL_MOB && status_get_hp(bl) < status_get_max_hp(bl))
    {
        p.maxHP = status_get_max_hp(bl);
        p.HP = status_get_hp(bl);
        p.isBoss = (((TBL_MOB*)bl)->spawn && ((TBL_MOB*)bl)->spawn->state.boss) ? 1 : 0;
    }
    else
    {
        p.maxHP = -1;
        p.HP = -1;
        p.isBoss = 0;
    }

    clif->send(&p,sizeof(p), tsd ? &tsd->bl : bl, target);

    if (disguised(bl))
    {
        p.objecttype = pc->db_checkid(status->get_viewdata(bl)->class) ? 0x0 : 0x5; //PC_TYPE : NPC_MOB_TYPE
        p.GID = -bl->id;
        clif->send(&p,sizeof(p),bl,SELF);
    }

}

void eclif_spawn_unit_old(struct block_list* bl, enum send_target target)
{
    struct map_session_data* sd;
    struct status_change* sc = status->get_sc(bl);
    struct view_data* vd = status->get_viewdata(bl);
    struct packet_spawn_unit_old p;
    int g_id = status->get_guild_id(bl);

    nullpo_retv(bl);

    sd = BL_CAST(BL_PC, bl);

    p.PacketType = 0x90f;
    p.PacketLength = sizeof(p);
    p.objecttype = clif_bl_type_old(bl);
//    p.AID = bl->id;
//    p.GID = (sd) ? sd->status.char_id : 0;    // CCODE
    p.GID = bl->id;
    p.speed = status->get_speed(bl);
    p.bodyState = (sc) ? sc->opt1 : 0;
    p.healthState = (sc) ? sc->opt2 : 0;
    p.effectState = (sc) ? sc->option : bl->type == BL_NPC ? ((TBL_NPC*)bl)->option : 0;
    p.job = vd->class;
    p.head = vd->hair_style;
    p.weapon = vd->weapon;
    p.accessory = vd->head_bottom;
    p.accessory2 = vd->head_top;
    p.accessory3 = vd->head_mid;
    if (bl->type == BL_NPC && vd->class == FLAG_CLASS)
    {   //The hell, why flags work like this?
        p.accessory = status->get_emblem_id(bl);
        p.accessory2 = GetWord(g_id, 1);
        p.accessory3 = GetWord(g_id, 0);
    }
    p.headpalette = vd->hair_color;
    p.bodypalette = vd->cloth_color;
    p.headDir = (sd)? sd->head_dir : 0;
    p.robe = vd->robe;
    p.GUID = g_id;
    p.GEmblemVer = status->get_emblem_id(bl);
    p.honor = (sd) ? sd->status.manner : 0;
    p.virtue = (sc) ? sc->opt3 : 0;
    p.isPKModeON = (sd && sd->status.karma) ? 1 : 0;
    p.sex = vd->sex;
    WBUFPOS(&p.PosDir[0],0,bl->x,bl->y,unit->getdir(bl));
    p.xSize = p.ySize = (sd) ? 5 : 0;
    p.clevel = clif_setlevel(bl);
    p.font = (sd) ? sd->status.font : 0;
    if (battle->bc->show_monster_hp_bar && bl->type == BL_MOB && status_get_hp(bl) < status_get_max_hp(bl))
    {
        p.maxHP = status_get_max_hp(bl);
        p.HP = status_get_hp(bl);
        p.isBoss = (((TBL_MOB*)bl)->spawn && ((TBL_MOB*)bl)->spawn->state.boss) ? 1 : 0;
    }
    else
    {
        p.maxHP = -1;
        p.HP = -1;
        p.isBoss = 0;
    }
    if (disguised(bl))
    {
        nullpo_retv(sd);
        if (sd->status.class != sd->disguise)
            clif->send(&p, sizeof(p), bl, target);
        p.objecttype = pc->db_checkid(status->get_viewdata(bl)->class) ? 0x0 : 0x5; //PC_TYPE : NPC_MOB_TYPE
        p.GID = -bl->id;
        clif->send(&p, sizeof(p), bl, SELF);
    }
    else
    {
        clif->send(&p, sizeof(p), bl, target);
    }
}

void eclif_set_unit_walking_old(struct block_list* bl,
                                struct map_session_data *tsd,
                                struct unit_data* ud,
                                enum send_target target)
{
    struct map_session_data* sd;
    struct status_change* sc = status->get_sc(bl);
    struct view_data* vd = status->get_viewdata(bl);
    struct packet_unit_walking_old p;
    int g_id = status->get_guild_id(bl);

    nullpo_retv(bl);
    nullpo_retv(ud);

    sd = BL_CAST(BL_PC, bl);

    p.PacketType = 0x914;
    p.PacketLength = sizeof(p);
    p.objecttype = clif_bl_type_old(bl);
//    p.AID = bl->id;
//    p.GID = (tsd) ? tsd->status.char_id : 0;    // CCODE
    p.GID = bl->id;
    p.speed = status->get_speed(bl);
    p.bodyState = (sc) ? sc->opt1 : 0;
    p.healthState = (sc) ? sc->opt2 : 0;
    p.effectState = (sc) ? sc->option : bl->type == BL_NPC ? ((TBL_NPC*)bl)->option : 0;
    p.job = vd->class;
    p.head = vd->hair_style;
    p.weapon = vd->weapon;
    p.accessory = vd->head_bottom;
    p.moveStartTime = (unsigned int)timer->gettick();
    p.accessory2 = vd->head_top;
    p.accessory3 = vd->head_mid;
    p.headpalette = vd->hair_color;
    p.bodypalette = vd->cloth_color;
    p.headDir = (sd)? sd->head_dir : 0;
    p.robe = vd->robe;
    p.GUID = g_id;
    p.GEmblemVer = status->get_emblem_id(bl);
    p.honor = (sd) ? sd->status.manner : 0;
    p.virtue = (sc) ? sc->opt3 : 0;
    p.isPKModeON = (sd && sd->status.karma) ? 1 : 0;
    p.sex = vd->sex;
    WBUFPOS2(&p.MoveData[0], 0, bl->x, bl->y, ud->to_x, ud->to_y, 8, 8);
    p.xSize = p.ySize = (sd) ? 5 : 0;
    p.clevel = clif_setlevel(bl);
    p.font = (sd) ? sd->status.font : 0;
    if (battle->bc->show_monster_hp_bar && bl->type == BL_MOB && status_get_hp(bl) < status_get_max_hp(bl))
    {
        p.maxHP = status_get_max_hp(bl);
        p.HP = status_get_hp(bl);
        p.isBoss = (((TBL_MOB*)bl)->spawn && ((TBL_MOB*)bl)->spawn->state.boss) ? 1 : 0;
    }
    else
    {
        p.maxHP = -1;
        p.HP = -1;
        p.isBoss = 0;
    }

    clif->send(&p, sizeof(p), tsd ? &tsd->bl : bl, target);

    if (disguised(bl))
    {
        p.objecttype = pc->db_checkid(status->get_viewdata(bl)->class) ? 0x0 : 0x5; //PC_TYPE : NPC_MOB_TYPE
        p.GID = -bl->id;
        clif->send(&p, sizeof(p), bl, SELF);
    }
}

void eclif_damage_old(struct block_list* src,
                      struct block_list* dst,
                      int sdelay,
                      int ddelay,
                      int64 in_damage,
                      short div,
                      unsigned char type,
                      int64 in_damage2)
{
    struct packet_damage_old p;
    struct status_change *sc;
    int damage,damage2;

    nullpo_retv(src);
    nullpo_retv(dst);

    sc = status->get_sc(dst);

    if (sc && sc->count && sc->data[SC_ILLUSION])
    {
        if(in_damage)
            in_damage = in_damage*(sc->data[SC_ILLUSION]->val2); //+ rnd()%100;
        if(in_damage2)
            in_damage2 = in_damage2*(sc->data[SC_ILLUSION]->val2); //+ rnd()%100;
    }

    damage = (int)min(in_damage,INT_MAX);
    damage2 = (int)min(in_damage2,INT_MAX);

    type = clif_calc_delay(type,div,damage+damage2,ddelay);

    p.PacketType = 0x2e1;
    p.GID = src->id;
    p.targetGID = dst->id;
    p.startTime = (uint32)timer->gettick();
    p.attackMT = sdelay;
    p.attackedMT = ddelay;
    p.count = div;
    p.action = type;

    if (battle->bc->hide_woe_damage && map_flag_gvg2(src->m))
    {
        p.damage = damage ? div : 0;
        p.leftDamage = damage2 ? div : 0;
    }
    else
    {
        p.damage = damage;
        p.leftDamage = damage2;
    }
//    p.is_sp_damaged = 0;    // [ToDo] IsSPDamage - Displays blue digits.

    if (disguised(dst))
    {
        clif->send(&p, sizeof(p), dst, AREA_WOS);
        p.targetGID = -dst->id;
        clif->send(&p, sizeof(p), dst, SELF);
    }
    else
    {
        clif->send(&p, sizeof(p), dst, AREA);
    }

    if (disguised(src))
    {
        p.GID = -src->id;
        if (disguised(dst))
            p.targetGID = dst->id;

        if(damage > 0)
            p.damage = -1;
        if(damage2 > 0)
            p.leftDamage = -1;

        clif->send(&p, sizeof(p), src, SELF);
    }

    if (src == dst)
    {
        unit->setdir(src, unit->getdir(src));
    }
}

void eclif_set_unit_idle_post(struct block_list *bl,
                              TBL_PC *tsd,
                              enum send_target target)
{
    if (!bl || !tsd)
        return;

    eclif_set_unit_idle_old(bl, tsd, target);

    if (bl->type == BL_MOB)
        send_mob_info(bl, &tsd->bl, target);
    else if (bl->type == BL_PC)
        send_pc_info(bl, &tsd->bl, target);
    else if (bl->type == BL_NPC)
        send_npc_info(bl, &tsd->bl, target);
}

void eclif_set_unit_walking_pre(struct block_list **blPtr,
                                TBL_PC **tsdPtr,
                                struct unit_data **udPtr,
                                enum send_target *target)
{
    eclif_set_unit_walking_old(*blPtr, *tsdPtr, *udPtr, *target);
}

void eclif_set_unit_walking_post(struct block_list *bl,
                                 TBL_PC *tsd,
                                 struct unit_data* ud,
                                 enum send_target target)
{
    if (!ud)
        return;
    TBL_PC *sd = BL_CAST(BL_PC, ud->bl);
    if (!sd || !pc_isinvisible(sd))
    {
        if (ud->walktimer != INVALID_TIMER)
            send_advmoving(ud, true, tsd ? &tsd->bl : bl, target);
        else
            send_advmoving(ud, false, tsd ? &tsd->bl : bl, target);
    }
}

int eclif_damage_post(int retVal,
                      struct block_list* src,
                      struct block_list* dst,
                      int sdelay,
                      int ddelay,
                      int64 in_damage,
                      short div,
                      unsigned char type,
                      int64 in_damage2)
{
    eclif_damage_old(src, dst,
        sdelay, ddelay, in_damage,
        div, type, in_damage2);
    return retVal;
}

void eclif_move_post(struct unit_data *ud)
{
    if (!ud)
        return;
    TBL_PC *sd = BL_CAST(BL_PC, ud->bl);
    if (!sd || !pc_isinvisible(sd))
        send_advmoving(ud, false, ud->bl, AREA_WOS);
}

void eclif_spawn_unit_pre(struct block_list **blPtr,
                          enum send_target *target)
{
    eclif_spawn_unit_old(*blPtr, *target);
}

bool tempChangeMap;

void eclif_parse_LoadEndAck_pre(int *fdPtr __attribute__ ((unused)),
                                struct map_session_data **sdPtr)
{
    struct map_session_data *sd = *sdPtr;
    if (!sd)
        return;
    sd->state.warp_clean = 0;
    tempChangeMap = sd->state.changemap;
}

void eclif_parse_LoadEndAck_post(int fd __attribute__ ((unused)),
                                 struct map_session_data *sd)
{
    if (!tempChangeMap)
    {   // some messages not sent if map not changed
        map->iwall_get(sd);
    }
    map_alwaysVisible_send(sd);
}

void eclif_changelook2(struct block_list *bl,
                       int type,
                       int val,
                       struct item_data *id,
                       int n)
{
    struct map_session_data* sd;
    struct status_change* sc;
    struct view_data* vd;
    enum send_target target = AREA;
    int val2 = 0;
    if (!bl)
        return;

    sd = BL_CAST(BL_PC, bl);
    sc = status->get_sc(bl);
    vd = status->get_viewdata(bl);

    if (vd)
    {
        switch(type)
        {
            case LOOK_WEAPON:
                if (sd)
                {
                    clif->get_weapon_view(sd, &vd->weapon, &vd->shield);
                    val = vd->weapon;
                }
                else
                {
                    vd->weapon = val;
                }
                break;

            case LOOK_SHIELD:
                if (sd)
                {
                    clif->get_weapon_view(sd, &vd->weapon, &vd->shield);
                    val = vd->shield;
                }
                else
                {
                    vd->shield = val;
                }
                break;
            case LOOK_BASE:
                if (!sd)
                    break;

                if (val == INVISIBLE_CLASS) /* nothing to change look */
                    return;

                if (sd->sc.option & OPTION_COSTUME)
                    vd->weapon = vd->shield = 0;

//                if (!vd->cloth_color)
//                    break;
            break;
            case LOOK_HAIR:
                vd->hair_style = val;
            break;
            case LOOK_HEAD_BOTTOM:
                vd->head_bottom = val;
            break;
            case LOOK_HEAD_TOP:
                vd->head_top = val;
            break;
            case LOOK_HEAD_MID:
                vd->head_mid = val;
            break;
            case LOOK_HAIR_COLOR:
                vd->hair_color = val;
            break;
            case LOOK_CLOTHES_COLOR:
                vd->cloth_color = val;
            break;
            case LOOK_SHOES:
//                if (sd) {
//                    int n;
//                    if((n = sd->equip_index[2]) >= 0 && sd->inventory_data[n]) {
//                        if(sd->inventory_data[n]->view_id > 0)
//                            val = sd->inventory_data[n]->view_id;
//                        else
//                            val = sd->status.inventory[n].nameid;
//                    } else
//                        val = 0;
//                }
            break;
            case LOOK_BODY:
            case LOOK_FLOOR:
                // unknown purpose
            break;
            case LOOK_ROBE:
                vd->robe = val;
            break;
        }
    }

    // prevent leaking the presence of GM-hidden objects
    if (sc && sc->option&OPTION_INVISIBLE && !( bl->type == BL_PC && ((TBL_PC*)bl)->disguise != -1))
        target = SELF;
    if (type == LOOK_WEAPON || type == LOOK_SHIELD)
    {
        if (!vd)
            return;
        type = LOOK_WEAPON;
        val = vd->weapon;
        val2 = vd->shield;
    }
    if ((bl->type == BL_PC && ((TBL_PC*)bl)->disguise != -1))
    {
        send_changelook2(sd, bl, bl->id, type, val, val2, id, n, AREA_WOS);
        send_changelook2(sd, bl, -bl->id, type, val, val2, id, n, SELF);
    }
    else
    {
        send_changelook2(sd, bl, bl->id, type, val, val2, id, n, target);
    }
}

static inline int itemtype(const int type)
{
    switch (type)
    {
#if PACKETVER >= 20080827
        case IT_WEAPON:
            return IT_ARMOR;
        case IT_ARMOR:
        case IT_PETARMOR:
#endif
        case IT_PETEGG:
            return IT_WEAPON;
        default:
            return type;
    }
}

void eclif_getareachar_item_pre(struct map_session_data **sdPtr,
                                struct flooritem_data **fitemPtr)
{
    int view;
    struct map_session_data *sd = *sdPtr;
    struct flooritem_data *fitem = *fitemPtr;
    if (!sd || !fitem)
        return;
    int fd = sd->fd;

    struct SessionExt *data = session_get(fd);
    if (!data || data->clientVersion < 10)
        return;

    WFIFOHEAD(fd, 28);
    WFIFOW(fd, 0) = 0xb18;
    WFIFOL(fd, 2) = fitem->bl.id;
    if((view = itemdb_viewid(fitem->item_data.nameid)) > 0)
        WFIFOW(fd, 6) = view;
    else
        WFIFOW(fd, 6) = fitem->item_data.nameid;
    WFIFOB(fd, 8) = itemtype(itemdb_type(fitem->item_data.nameid));
    WFIFOB(fd, 9) = fitem->item_data.identify;
    WFIFOB(fd, 10) = fitem->item_data.attribute;
    WFIFOB(fd, 11) = fitem->item_data.refine;
    clif->addcards(WFIFOP(fd, 12), &fitem->item_data);
    WFIFOW(fd, 20) = fitem->bl.x;
    WFIFOW(fd, 22) = fitem->bl.y;
    WFIFOW(fd, 24) = fitem->item_data.amount;
    WFIFOB(fd, 26) = fitem->subx;
    WFIFOB(fd, 27) = fitem->suby;
    WFIFOSET(fd, 28);
    hookStop();
}

void eclif_dropflooritem_pre(struct flooritem_data **fitemPtr)
{
    char buf[28];
    int view;
    struct flooritem_data *fitem = *fitemPtr;

    if (!fitem)
        return;
    struct ItemdExt *itemData = itemd_get_by_item(&fitem->item_data);
    if (itemData)
    {
        if (itemData->subX)
            fitem->subx = (rand() % (itemData->subX * 2)) - itemData->subX;
        else
            fitem->subx = 0;
        if (itemData->subY)
            fitem->suby = (rand() % (itemData->subY * 2)) - itemData->subY;
        else
            fitem->suby = 0;
    }

    WBUFW(buf, 0) = 0xb19;
    WBUFL(buf, 2) = fitem->bl.id;
    if((view = itemdb_viewid(fitem->item_data.nameid)) > 0)
        WBUFW(buf, 6) = view;
    else
        WBUFW(buf, 6) = fitem->item_data.nameid;
    WBUFB(buf, 8) = itemtype(itemdb_type(fitem->item_data.nameid));
    WBUFB(buf, 9) = fitem->item_data.identify;
    WBUFB(buf, 10) = fitem->item_data.attribute;
    WBUFB(buf, 11) = fitem->item_data.refine;
    clif->addcards(WBUFP(buf, 12), &fitem->item_data);
    WBUFW(buf, 20) = fitem->bl.x;
    WBUFW(buf, 22) = fitem->bl.y;
    WBUFW(buf, 24) = fitem->item_data.amount;
    WBUFB(buf, 26) = fitem->subx;
    WBUFB(buf, 27) = fitem->suby;

    clif->send(&buf, 28, &fitem->bl, AREA);
}

void eclif_sendbgemblem_area_pre(struct map_session_data **sdPtr)
{
    unsigned char buf[34];
    struct map_session_data *sd = *sdPtr;
    struct SessionExt *data = session_get_bysd(sd);
    if (!sd || !data || data->clientVersion < 12)
        return;

    WBUFW(buf, 0) = 0xb1a;
    WBUFL(buf, 2) = sd->bl.id;
    safestrncpy(WBUFP(buf,6), sd->status.name, NAME_LENGTH); // name don't show in screen.
    WBUFW(buf, 30) = sd->bg_id;
    WBUFW(buf, 32) = data->teamId;
    clif->send(buf, 34, &sd->bl, AREA);
}

void eclif_sendbgemblem_single_pre(int *fdPtr,
                                   struct map_session_data **sdPtr)
{
    int fd = *fdPtr;
    struct map_session_data *sd = *sdPtr;
    struct SessionExt *data = session_get_bysd(sd);
    struct SessionExt *ddata = session_get_bysd(sd);
    if (!sd || !data || !ddata || ddata->clientVersion < 12)
        return;

    WFIFOHEAD(fd, 34);
    WFIFOW(fd, 0) = 0xb1a;
    WFIFOL(fd, 2) = sd->bl.id;
    safestrncpy(WFIFOP(fd, 6), sd->status.name, NAME_LENGTH);
    WFIFOW(fd, 30) = sd->bg_id;
    WFIFOW(fd, 32) = data->teamId;
    WFIFOSET(fd, 34);
    hookStop();
    return;
}

void eclif_disp_message_pre(struct block_list **srcPtr,
                            const char **mesPtr,
                            enum send_target *targetPtr)
{
    unsigned char buf[256];
    struct block_list *src = *srcPtr;
    const char *mes = *mesPtr;

    nullpo_retv(mes);

    int len = (int)strlen(mes);

    if (len == 0 || !isInit)
        return;

    nullpo_retv(src);

    if (len > sizeof(buf) - 5)
    {
        ShowWarning("clif_disp_message: Truncated message '%s' (len=%d, max=%d, aid=%d).\n", mes, (int)len, (int)(sizeof(buf) - 5), src->id);
        len = sizeof(buf) - 5;
    }

    WBUFW(buf, 0) = 0x8e;
    WBUFW(buf, 2) = len + 5;
    safestrncpy(WBUFP(buf, 4), mes, len + 1);
    clif->send(buf, WBUFW(buf, 2), src, *targetPtr);
}

void eclif_addcards_post(unsigned char *buf, struct item *item)
{
    if (!buf || !item)
        return;
    if (item->card[0] == CARD0_PET)
    {
        WBUFW(buf, 0) = item->card[0];
        WBUFW(buf, 2) = item->card[1];
        WBUFW(buf, 4) = item->card[2];
        WBUFW(buf, 6) = item->card[3];
    }
}

void eclif_addcards2_post(unsigned short *cards, struct item *item)
{
    if (!cards || !item)
        return;
    if (item->card[0] == CARD0_PET)
    {
        cards[0] = item->card[0];
        cards[1] = item->card[1];
        cards[2] = item->card[2];
        cards[3] = item->card[3];
    }
}

void eclif_useskill(struct block_list* bl,
                    int src_id,
                    int dst_id,
                    int dst_x,
                    int dst_y,
                    uint16 skill_id,
                    uint16 skill_lv,
                    int casttime)
{
    const int cmd = 0x7fb;
    unsigned char buf[50];
    int property = skill->get_ele(skill_id, skill_lv);

    // for client < 18
    WBUFW(buf, 0) = cmd;
    WBUFL(buf, 2) = src_id;
    WBUFL(buf, 6) = dst_id;
    WBUFW(buf, 10) = dst_x;
    WBUFW(buf, 12) = dst_y;
    WBUFW(buf, 14) = skill_id;
    WBUFL(buf, 16) = property < 0 ? 0 : property; //Avoid sending negatives as element [Skotlex]
    WBUFL(buf, 20) = casttime;
    WBUFB(buf, 24) = 0;  // isDisposable

    if (clif->isdisguised(bl))
    {
        clif->send(buf, 25, bl, AREA_WOS);
        WBUFL(buf, 2) = -src_id;
        clif->send(buf, 25, bl, SELF);
    }
    else
    {
        clif->send(buf, 25, bl, AREA);
    }

    // for client >= 18
    const int len = 36;
    WBUFW(buf, 0) = 0xb1e;
    WBUFW(buf, 2) = len;
    WBUFL(buf, 4) = src_id;
    WBUFL(buf, 8) = dst_id;
    WBUFW(buf, 12) = dst_x;
    WBUFW(buf, 14) = dst_y;
    WBUFW(buf, 16) = skill_id;
    WBUFW(buf, 18) = skill_lv;
    WBUFL(buf, 20) = property < 0 ? 0 : property; //Avoid sending negatives as element [Skotlex]
    WBUFL(buf, 24) = casttime;
    WBUFL(buf, 28) = skill->get_splash(skill_id, skill_lv);
    WBUFL(buf, 32) = skill->get_inf2(skill_id);

    if (clif->isdisguised(bl))
    {
        clif->send(buf, len, bl, AREA_WOS);
        WBUFL(buf, 2) = -src_id;
        clif->send(buf, len, bl, SELF);
    }
    else
    {
        clif->send(buf, len, bl, AREA);
    }
}

void eclif_skillinfoblock_pre(struct map_session_data **sdPtr)
{
    struct map_session_data *sd = *sdPtr;
    nullpo_retv(sd);
    struct SessionExt *data = session_get_bysd(sd);
    if (!data)
        return;
    if (data->clientVersion < 18)
        return;

    int fd = sd->fd;
    if (!fd)
        return;

    WFIFOHEAD(fd, MAX_SKILL * 41 + 4);
    WFIFOW(fd, 0) = 0x10f;
    int len = 4;
    int i;
    for (i = 0; i < MAX_SKILL; i++)
    {
        int id = sd->status.skill[i].id;
        if (id != 0)
        {
            const int skillSize = 41;
            if (len + skillSize > 16384)
                break;

            WFIFOW(fd, len) = id;
            WFIFOL(fd, len + 2) = skill->get_inf(id);
            WFIFOL(fd, len + 6) = skill->get_inf2(id);
            const int level = sd->status.skill[i].lv;
            WFIFOW(fd, len + 10) = level;
            if (level)
            {
                WFIFOW(fd, len + 12) = skill->get_sp(id, level);
                WFIFOW(fd, len + 14) = skill->get_range2(&sd->bl, id, level);
            }
            else
            {
                WFIFOW(fd, len + 12) = 0;
                WFIFOW(fd, len + 14) = 0;
            }
            safestrncpy(WFIFOP(fd, len + 16), skill->get_name(id), NAME_LENGTH);
            if (sd->status.skill[i].flag == SKILL_FLAG_PERMANENT)
                WFIFOB(fd, len + 40) = (sd->status.skill[i].lv < skill->tree_get_max(id, sd->status.class)) ? 1 : 0;
            else
                WFIFOB(fd, len + 40) = 0;
            len += skillSize;
        }
    }
    WFIFOW(fd, 2) = len;
    WFIFOSET(fd, len);

    // workaround for bugreport:5348; send the remaining skills one by one to bypass packet size limit
    for ( ; i < MAX_SKILL; i++)
    {
        int id = sd->status.skill[i].id;
        if (id != 0)
        {
            clif->addskill(sd, id);
            clif->skillinfo(sd, id, 0);
        }
    }
    hookStop();
}

void eclif_addskill_pre(struct map_session_data **sdPtr,
                        int *idPtr)
{
    struct map_session_data *sd = *sdPtr;
    nullpo_retv(sd);
    int id = *idPtr;

    struct SessionExt *data = session_get_bysd(sd);
    if (!data)
        return;
    if (data->clientVersion < 18)
        return;

    int fd = sd->fd;
    if (!fd)
    {
        hookStop();
        return;
    }

    const int idx = skill->get_index(id);

    if (sd->status.skill[idx].id <= 0)
    {
        hookStop();
        return;
    }

    const int skill_lv = sd->status.skill[idx].lv;
    const int sz = 45;

    WFIFOHEAD(fd, sz);
    WFIFOW(fd, 0) = 0xb1f;
    WFIFOW(fd, 2) = sz;
    WFIFOW(fd, 4) = id;
    WFIFOL(fd, 6) = skill->get_inf(id);
    WFIFOL(fd, 10) = skill->get_inf2(id);
    WFIFOW(fd, 14) = skill_lv;
    if (skill_lv > 0)
    {
        WFIFOW(fd, 16) = skill->get_sp(id, skill_lv);
        WFIFOW(fd, 18) = skill->get_range2(&sd->bl, id, skill_lv);
    }
    else
    {
        WFIFOW(fd, 16) = 0;
        WFIFOW(fd, 18) = 0;
    }
    safestrncpy(WFIFOP(fd, 20), skill->get_name(id), NAME_LENGTH);
    if (sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT)
        WFIFOB(fd, 44) = (skill_lv < skill->tree_get_max(id, sd->status.class)) ? 1 : 0;
    else
        WFIFOB(fd, 44) = 0;
    WFIFOSET(fd, sz);

    hookStop();
}

void eclif_skillinfo_pre(struct map_session_data **sdPtr,
                         int *skill_idPtr,
                         int *infPtr)
{
    struct map_session_data *sd = *sdPtr;
    nullpo_retv(sd);

    struct SessionExt *data = session_get_bysd(sd);
    if (!data)
        return;
    if (data->clientVersion < 18)
        return;

    int skill_id = *skill_idPtr;
    int idx = skill->get_index(skill_id);
    Assert_retv(idx >= 0 && idx < MAX_SKILL);
    int inf = *infPtr;

    const int fd = sd->fd;
    int skill_lv = sd->status.skill[idx].lv;

    const int sz = 21;
    WFIFOHEAD(fd, sz);
    WFIFOW(fd, 0) = 0xb20;
    WFIFOW(fd, 2) = sz;
    WFIFOW(fd, 4) = skill_id;
    WFIFOL(fd, 6) = inf ? inf : skill->get_inf(skill_id);
    WFIFOL(fd, 10) = skill->get_inf2(skill_id);
    WFIFOW(fd, 14) = skill_lv;
    if (skill_lv > 0) {
        WFIFOW(fd, 16) = skill->get_sp(skill_id, skill_lv);
        WFIFOW(fd, 18) = skill->get_range2(&sd->bl, skill_id, skill_lv);
    } else {
        WFIFOW(fd, 16) = 0;
        WFIFOW(fd, 18) = 0;
    }
    if (sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT)
        WFIFOB(fd, 20) = (skill_lv < skill->tree_get_max(skill_id, sd->status.class)) ? 1 : 0;
    else
        WFIFOB(fd, 20) = 0;
    WFIFOSET(fd, sz);
}

void eclif_parse_WalkToXY(int fd,
                          struct map_session_data *sd)
{
    short x, y;

    if (pc_isdead(sd))
    {
        clif->clearunit_area(&sd->bl, CLR_DEAD);
        return;
    }

    if (sd->sc.opt1 && (sd->sc.opt1 == OPT1_STONEWAIT || sd->sc.opt1 == OPT1_BURNING))
        ; //You CAN walk on this OPT1 value.
    /*else if( sd->progressbar.npc_id )
        clif->progressbar_abort(sd);*/
    else if (pc_cant_act(sd))
        return;

    if(sd->sc.data[SC_RUN] || sd->sc.data[SC_WUGDASH])
        return;

    pc->delinvincibletimer(sd);

    RFIFOPOS(fd, 2, &x, &y, NULL);

    //Set last idle time... [Skotlex]
    pc->update_idle_time(sd, BCIDLE_WALK);

    if (sd->ud.state.change_walk_target == 0)
    {
        if (unit->walktoxy(&sd->bl, x, y, 4) &&
            sd->ud.state.change_walk_target == 1)
        {
            struct SessionExt *data = session_get_bysd(sd);
            if (!data)
                return;
            if (data->clientVersion < 18)
                return;
            send_walk_fail(sd->fd, x, y);
        }
    }
    else
    {
        unit->walktoxy(&sd->bl, x, y, 4);
    }
}

void eclif_party_info_post(struct party_data *p,
                           struct map_session_data *sd)
{
    if (sd)
    {
        clif->party_option(p, sd, 2);
    }
}

/// NPC text input dialog value (CZ_INPUT_EDITDLGSTR).
/// 01d5 <packet len>.W <npc id>.L <string>.?B
void eclif_parse_NpcStringInput(int fd,
                                struct map_session_data* sd)
{
    int message_len = RFIFOW(fd, 2) - 8;
    int npcid = RFIFOL(fd, 4);
    const char *message = RFIFOP(fd, 8);

    if (message_len <= 0)
        return; // invalid input

    if (message_len > 1000)
        message_len = 1000;

    safestrncpy(global_npc_str, message, message_len);
    npc->scriptcont(sd, npcid, false);
}
