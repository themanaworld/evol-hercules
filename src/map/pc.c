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

int epc_readparam_pre(TBL_PC* sd, int *type)
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

int epc_setregistry(TBL_PC *sd, int64 *reg, int *val)
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

void epc_equipitem_pos(TBL_PC *sd, struct item_data *id, int *posPtr)
{
    int pos = *posPtr;

    if (!id)
        return;

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

void epc_unequipitem_pos(TBL_PC *sd,
                         int *nPtr __attribute__ ((unused)),
                         int *posPtr)
{
    if (!sd)
        return;

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

bool epc_can_attack (TBL_PC *sd, int *target_id)
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

int epc_takeitem(TBL_PC *sd __attribute__ ((unused)),
                 TBL_ITEM *fitem)
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

void epc_validate_levels(void)
{
    int i;
    for (i = 0; i < 7; i++) {
        if (!pc->db_checkid(i)) continue;
        if (i == JOB_WEDDING || i == JOB_XMAS || i == JOB_SUMMER)
            continue; //Classes that do not need exp tables.
        int j = pc->class2idx(i);
        if (!pc->max_level[j][0])
            ShowWarning("Class %d does not has a base exp table.\n", i);
        if (!pc->max_level[j][1])
            ShowWarning("Class %d does not has a job exp table.\n", i);
    }
    hookStop();
}

int epc_isuseequip_post(int retVal, struct map_session_data *sd, int *nPtr)
{
    const int n = *nPtr;
    if (retVal)
    {
        if (!sd)
            return 0;

        if (n < 0 || n >= MAX_INVENTORY)
            return 0;

        struct ItemdExt *data = itemd_get(sd->inventory_data[n]);
        if (!data)
            return retVal;

        if (sd->battle_status.str < data->requiredStr ||
            sd->battle_status.agi < data->requiredAgi ||
            sd->battle_status.vit < data->requiredVit ||
            sd->battle_status.int_ < data->requiredInt ||
            sd->battle_status.dex < data->requiredDex ||
            sd->battle_status.luk < data->requiredLuk ||
            sd->battle_status.max_hp < data->requiredMaxHp ||
            sd->battle_status.max_sp < data->requiredMaxSp ||
            sd->battle_status.batk < data->requiredAtk ||
            sd->battle_status.matk_min < data->requiredMAtkMin ||
            sd->battle_status.matk_max < data->requiredMAtkMax ||
            sd->battle_status.def < data->requiredDef ||
            sd->battle_status.mdef < data->requiredMDef
        )
        {
            return 0;
        }
    }
    return retVal;
}

int epc_useitem_post(int retVal, struct map_session_data *sd, int *nPtr)
{
    const int n = *nPtr;
    if (!sd)
        return retVal;

    if (n < 0 || n >= MAX_INVENTORY)
        return retVal;

    struct ItemdExt *data = itemd_get(sd->inventory_data[n]);
    if (!data)
        return retVal;

    const int effect = retVal ? data->useEffect : data->useFailEffect;
    if (effect != -1)
        clif->specialeffect(&sd->bl, effect, AREA);
    return retVal;
}

static void equippost_effect(struct map_session_data *const sd, const int n, const bool retVal, const bool equip)
{
    if (!sd)
        return;

    if (n < 0 || n >= MAX_INVENTORY)
        return;

    struct ItemdExt *data = itemd_get(sd->inventory_data[n]);
    if (!data)
        return;

    int effect;
    if (equip)
        effect = retVal ? data->useEffect : data->useFailEffect;
    else
        effect = retVal ? data->unequipEffect : data->unequipFailEffect;

    if (effect != -1)
        clif->specialeffect(&sd->bl, effect, AREA);
    return;
}

int epc_equipitem_post(int retVal, struct map_session_data *sd,
                       int *nPtr, int *data __attribute__ ((unused)))
{
    equippost_effect(sd, *nPtr, retVal, true);
    return retVal;
}

int epc_unequipitem_post(int retVal, struct map_session_data *sd,
                         int *nPtr, int *data __attribute__ ((unused)))
{
    equippost_effect(sd, *nPtr, retVal, false);
    return retVal;
}

int epc_check_job_name(const char *name)
{
    int val = -1;
    if (script->get_constant(name, &val))
    {
        hookStop();
        return val;
    }
    hookStop();
    return -1;
}
