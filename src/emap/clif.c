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
#include "common/timer.h"
#include "map/guild.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/quest.h"

#include "emap/clif.h"
#include "emap/lang.h"
#include "emap/send.h"
#include "emap/data/mapd.h"
#include "emap/data/session.h"
#include "emap/struct/mapdext.h"
#include "emap/struct/sessionext.h"

extern bool isInit;

void eclif_quest_send_list(TBL_PC *sd)
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

void eclif_quest_add(TBL_PC *sd, struct quest *qd)
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

    WFIFOHEAD(fd, 107);
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
        TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
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
            send_changelook(sd, sd2, fd, id, field, item->look, 0, item, equip); \
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

#undef equipPos

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
    //skipping SHADOW slots
}

#undef equipPos2

void eclif_getareachar_unit_post(TBL_PC* sd, struct block_list *bl)
{
    if (!bl)
        return;
    if (bl->type == BL_PC)
    {
        eclif_send_additional_slots((TBL_PC *)bl, sd);
        send_pc_info(bl, &sd->bl, SELF);
    }
}

bool eclif_spawn_post(bool retVal, struct block_list *bl)
{
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

void eclif_changemap_post(TBL_PC *sd, short *m,
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

void eclif_set_unit_idle(struct block_list* bl, TBL_PC *tsd, enum send_target *target)
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
    }
    return 0;
}

void eclif_set_unit_idle_post(struct block_list* bl, TBL_PC *tsd,
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

void eclif_set_unit_walking(struct block_list* bl, TBL_PC *tsd,
                            struct unit_data* ud, enum send_target *target)
{
    TBL_PC *sd = BL_CAST(BL_PC, ud->bl);
    if (!sd || !pc_isinvisible(sd))
    {
        if (ud->walktimer != INVALID_TIMER)
            send_advmoving(ud, true, tsd ? &tsd->bl : bl, *target);
        else
            send_advmoving(ud, false, tsd ? &tsd->bl : bl, *target);
    }
}

void eclif_move(struct unit_data *ud)
{
    TBL_PC *sd = BL_CAST(BL_PC, ud->bl);
    if (!sd || !pc_isinvisible(sd))
        send_advmoving(ud, false, ud->bl, AREA_WOS);
}

bool tempChangeMap;

void eclif_parse_LoadEndAck_pre(int *fdPtr __attribute__ ((unused)),
                                struct map_session_data *sd)
{
    sd->state.warp_clean = 0;
    tempChangeMap = sd->state.changemap;
}

void eclif_parse_LoadEndAck_post(int *fdPtr __attribute__ ((unused)),
                                 struct map_session_data *sd)
{
    if (!tempChangeMap)
    {   // some messages not sent if map not changed
        map->iwall_get(sd);
    }
}

void eclif_changelook2(struct block_list *bl, int type, int val,
                       struct item_data *id, int n)
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

void eclif_getareachar_item(struct map_session_data *sd, struct flooritem_data *fitem)
{
    int view;
    int fd = sd->fd;

    struct SessionExt *data = session_get(fd);
    if (!data || data->clientVersion < 10)
        return;
    hookStop();
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
}

void eclif_dropflooritem(struct flooritem_data* fitem)
{
    char buf[28];
    int view;

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

void eclif_sendbgemblem_area(struct map_session_data *sd)
{
    unsigned char buf[34];
    struct SessionExt *data = session_get_bysd(sd);
    if (!sd || !data || data->clientVersion < 12)
        return;

    WBUFW(buf, 0) = 0xb1a;
    WBUFL(buf, 2) = sd->bl.id;
    safestrncpy((char*)WBUFP(buf,6), sd->status.name, NAME_LENGTH); // name don't show in screen.
    WBUFW(buf, 30) = sd->bg_id;
    WBUFW(buf, 32) = data->teamId;
    clif->send(buf, 34, &sd->bl, AREA);
}

void eclif_sendbgemblem_single(int *fdPtr, struct map_session_data *sd)
{
    int fd = *fdPtr;
    struct SessionExt *data = session_get_bysd(sd);
    struct SessionExt *ddata = session_get_bysd(sd);
    if (!sd || !data || !ddata || ddata->clientVersion < 12)
        return;

    WFIFOHEAD(fd, 34);
    WFIFOW(fd, 0) = 0xb1a;
    WFIFOL(fd, 2) = sd->bl.id;
    safestrncpy((char*)WFIFOP(fd, 6), sd->status.name, NAME_LENGTH);
    WFIFOW(fd, 30) = sd->bg_id;
    WFIFOW(fd, 32) = data->teamId;
    WFIFOSET(fd, 34);
    hookStop();
    return;
}

void eclif_disp_message(struct block_list* src,
                        const char* mes, size_t *lenPtr,
                        enum send_target *targetPtr)
{
    unsigned char buf[256];

    int len = *lenPtr;

    if (len == 0 || !isInit)
        return;

    nullpo_retv(src);
    nullpo_retv(mes);

    if (len > sizeof(buf) - 5)
    {
        ShowWarning("clif_disp_message: Truncated message '%s' (len=%d, max=%d, aid=%d).\n", mes, (int)len, (int)(sizeof(buf) - 5), src->id);
        len = sizeof(buf) - 5;
    }

    WBUFW(buf, 0) = 0x8e;
    WBUFW(buf, 2) = len + 5;
    safestrncpy((char*)WBUFP(buf, 4), mes, len + 1);
    clif->send(buf, WBUFW(buf, 2), src, *targetPtr);
}
