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
#include "../../../map/itemdb.h"
#include "../../../map/pc.h"

#include "map/pc.h"
#include "map/data/itemd.h"
#include "map/data/mapd.h"
#include "map/data/session.h"
#include "map/struct/itemdext.h"
#include "map/struct/mapdext.h"
#include "map/struct/sessionext.h"

int langScriptId;

int epc_readparam_pre(struct map_session_data* sd, int *type)
{
    if (*type == Const_ClientVersion)
    {
        struct SessionExt *data = session_get_bysd(sd);
        hookStop();
        if (!data)
            return 0;
        return data->clientVersion;
    }
    return 0;
}

int epc_setregistry(struct map_session_data *sd, int64 *reg, int *val)
{
    if (*reg == langScriptId)
    {
        struct SessionExt *data = session_get_bysd(sd);
        if (!data)
            return 0;

        data->language = *val;
    }

    return 0;
}

#define equipPos(mask, field, lookf) \
    if (pos & mask) \
    { \
        if (id) \
            sd->status.field = id->look; \
        else \
            sd->status.field = 0; \
        clif->changelook(&sd->bl, lookf, sd->status.field); \
        hookStop(); \
    }

#define equipPos2(mask, lookf) \
    if (pos & mask) \
    { \
        if (id) \
            clif->changelook(&sd->bl, lookf, id->look); \
        else \
            clif->changelook(&sd->bl, lookf, 0); \
        hookStop(); \
    }

void epc_equipitem_pos(struct map_session_data *sd, struct item_data *id, int *posPtr)
{
    int pos = *posPtr;

    equipPos(EQP_HEAD_LOW, head_bottom, LOOK_HEAD_BOTTOM);
    equipPos(EQP_HEAD_TOP, head_top, LOOK_HEAD_TOP);
    equipPos(EQP_HEAD_MID, head_mid, LOOK_HEAD_MID);
    equipPos(EQP_GARMENT, robe, LOOK_ROBE);
    //skip EQP_ARMOR
    equipPos2(EQP_SHOES, LOOK_SHOES);
    equipPos2(EQP_COSTUME_HEAD_TOP, 13);
    equipPos2(EQP_COSTUME_HEAD_MID, 14);
    equipPos2(EQP_COSTUME_HEAD_LOW, 15);
    equipPos2(EQP_COSTUME_GARMENT, 16);
    equipPos2(EQP_ARMOR, 17);
    //skipping SHADOW slots
}

#undef equipPos
#undef equipPos2

#define unequipPos(mask, field, lookf) \
    if (pos & mask) \
    { \
        sd->status.field = 0; \
        clif->changelook(&sd->bl, lookf, sd->status.field); \
        hookStop(); \
    }

#define unequipPos2(mask, lookf) \
    if (pos & mask) \
    { \
        clif->changelook(&sd->bl, lookf, 0); \
        hookStop(); \
    }

void epc_unequipitem_pos(struct map_session_data *sd,
                         int *nPtr __attribute__ ((unused)),
                         int *posPtr)
{
    int pos = *posPtr;

    unequipPos(EQP_HEAD_LOW, head_bottom, LOOK_HEAD_BOTTOM);
    unequipPos(EQP_HEAD_TOP, head_top, LOOK_HEAD_TOP);
    unequipPos(EQP_HEAD_MID, head_mid, LOOK_HEAD_MID);
    unequipPos(EQP_GARMENT, robe, LOOK_ROBE);
    unequipPos2(EQP_SHOES, LOOK_SHOES);
    unequipPos2(EQP_COSTUME_HEAD_TOP, 13);
    unequipPos2(EQP_COSTUME_HEAD_MID, 14);
    unequipPos2(EQP_COSTUME_HEAD_LOW, 15);
    unequipPos2(EQP_COSTUME_GARMENT, 16);
    unequipPos2(EQP_ARMOR, 17);
    //skipping SHADOW slots
}

#undef unequipPos
#undef unequipPos2

bool epc_can_attack (struct map_session_data *sd, int *target_id)
{
    if (!sd)
        return false;

    struct MapdExt *data = mapd_get(sd->bl.m);
    if (!data)
        return true;
    if (data->flag.nopve)
    {
        if (map->id2md(*target_id))
        {
            hookStop();
            return false;
        }
    }
    return true;
}

int epc_takeitem(struct map_session_data *sd __attribute__ ((unused)),
                 struct flooritem_data *fitem)
{
    if (!fitem)
        return 0;

    struct ItemdExt *data = itemd_get_by_item(&fitem->item_data);
    if (!data)
        return 1;

    if (!data->allowPickup)
    {
        hookStop();
        return 0;
    }
    return 1;
}

int epc_class2idx(int *classPtr)
{
    int class_ = *classPtr;
    if (class_ >= 30)
        class_ -= 30;
    if (class_ < 0)
        class_ = 0;
    if (class_ >= CLASS_COUNT)
        class_ = 0;
    return class_;
}

int epc_jobid2mapid(unsigned short *b_class)
{
    hookStop();
    return *b_class;
}

int epc_mapid2jobid(unsigned short *class_, int *sex __attribute__ ((unused)))
{
    hookStop();
    return *class_;
}

bool epc_db_checkid(unsigned int *class_)
{
    if (*class_ >= 30 && *class_ < 100)
    {
        hookStop();
        return true;
    }
    return false;
}

int epc_calc_skilltree_normalize_job(struct map_session_data *sd)
{
    hookStop();
    if (!sd)
        return 0;
    return sd->class_;
}
