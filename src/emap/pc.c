// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/itemdb.h"
#include "map/pc.h"

#include "emap/clif.h"
#include "emap/pc.h"
#include "emap/data/itemd.h"
#include "emap/data/mapd.h"
#include "emap/data/session.h"
#include "emap/struct/itemdext.h"
#include "emap/struct/mapdext.h"
#include "emap/struct/sessionext.h"

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
    if (pos & (mask)) \
    { \
        if (id) \
            sd->status.field = id->look; \
        else \
            sd->status.field = 0; \
        eclif_changelook2(&sd->bl, lookf, sd->status.field, id, n); \
    }

#define equipPos2(mask, lookf) \
    if (pos & (mask)) \
    { \
        if (id) \
            eclif_changelook2(&sd->bl, lookf, id->look, id, n); \
        else \
            eclif_changelook2(&sd->bl, lookf, 0, id, n); \
    }

void epc_equipitem_pos(TBL_PC *sd, struct item_data *id, int *nPtr, int *posPtr)
{
    const int n = *nPtr;
    int pos = *posPtr;

    hookStop();

    if (!id)
        return;

    if (pos & (EQP_HAND_R|EQP_SHADOW_WEAPON))
    {
        if(id)
        {
            sd->weapontype1 = id->look;
        }
        else
        {
            sd->weapontype1 = 0;
        }
        pc->calcweapontype(sd);
        eclif_changelook2(&sd->bl, LOOK_WEAPON, sd->status.weapon, id, n);
    }
    if (pos & (EQP_HAND_L|EQP_SHADOW_SHIELD))
    {
        if (id)
        {
            if(id->type == IT_WEAPON)
            {
                sd->status.shield = 0;
                sd->weapontype2 = id->look;
            }
            else if (id->type == IT_ARMOR)
            {
                sd->status.shield = id->look;
                sd->weapontype2 = 0;
            }
        }
        else
        {
            sd->status.shield = sd->weapontype2 = 0;
        }
        pc->calcweapontype(sd);
        eclif_changelook2(&sd->bl, LOOK_SHIELD, sd->status.shield, id, n);
    }

    equipPos(EQP_HEAD_LOW, head_bottom, LOOK_HEAD_BOTTOM);
    equipPos(EQP_HEAD_TOP, head_top, LOOK_HEAD_TOP);
    equipPos(EQP_HEAD_MID, head_mid, LOOK_HEAD_MID);
    equipPos(EQP_GARMENT, robe, LOOK_ROBE);
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
    if (pos & (mask)) \
    { \
        sd->status.field = 0; \
        eclif_changelook2(&sd->bl, lookf, sd->status.field, 0, n); \
    }

#define unequipPos2(mask, lookf) \
    if (pos & (mask)) \
        eclif_changelook2(&sd->bl, lookf, 0, 0, n);

void epc_unequipitem_pos(TBL_PC *sd,
                         int *nPtr,
                         int *posPtr)
{
    if (!sd)
        return;

    hookStop();

    const int n = *nPtr;
    int pos = *posPtr;

    if (pos & EQP_HAND_R)
    {
        sd->weapontype1 = 0;
        sd->status.weapon = sd->weapontype2;
        pc->calcweapontype(sd);
        eclif_changelook2(&sd->bl, LOOK_WEAPON, sd->status.weapon, 0, n);
        if (!battle->bc->dancing_weaponswitch_fix)
            status_change_end(&sd->bl, SC_DANCING, INVALID_TIMER);
    }
    if (pos & EQP_HAND_L)
    {
        sd->status.shield = sd->weapontype2 = 0;
        pc->calcweapontype(sd);
        eclif_changelook2(&sd->bl, LOOK_SHIELD, sd->status.shield, 0, n);
    }

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

int epc_setnewpc(int retVal, struct map_session_data *sd,
                 int *account_id __attribute__ ((unused)),
                 int *char_id __attribute__ ((unused)),
                 int *login_id1 __attribute__ ((unused)),
                 unsigned int *client_tick  __attribute__ ((unused)),
                 int *sex __attribute__ ((unused)),
                 int *fd __attribute__ ((unused)))
{
    if (sd)
    {
        sd->battle_status.speed = 150;
        sd->base_status.speed = 150;
    }
    return retVal;
}

int epc_additem_post(int retVal, struct map_session_data *sd,
                     struct item *item_data,
                     int *amountPtr __attribute__ ((unused)),
                     e_log_pick_type *log_type __attribute__ ((unused)))
{
    if (!retVal)
    {
        struct ItemdExt *data = itemd_get_by_item(item_data);
        if (data && data->charmItem)
            status_calc_pc(sd, SCO_NONE);
    }
    return retVal;
}

static bool calcPc = false;

int epc_delitem_pre(struct map_session_data *sd,
                    int *nPtr, int *amountPtr,
                    int *typePtr __attribute__ ((unused)),
                    short *reasonPtr __attribute__ ((unused)),
                    e_log_pick_type *log_type __attribute__ ((unused)))
{
    if (!sd)
        return 1;
    const int n = *nPtr;
    const int amount = *amountPtr;

    if (sd->status.inventory[n].nameid == 0 ||
        amount <= 0 ||
        sd->status.inventory[n].amount < amount ||
        sd->inventory_data[n] == NULL)
    {
        return 1;
    }

    struct ItemdExt *data = itemd_get(sd->inventory_data[n]);
    if (data && data->charmItem)
        calcPc = true;

    return 0;
}

int epc_delitem_post(int retVal,
                     struct map_session_data *sd,
                     int *nPtr __attribute__ ((unused)),
                     int *amountPtr __attribute__ ((unused)),
                     int *typePtr __attribute__ ((unused)),
                     short *reasonPtr __attribute__ ((unused)),
                     e_log_pick_type *log_type __attribute__ ((unused)))
{
    if (!retVal && calcPc && sd)
        status_calc_pc(sd, SCO_NONE);
    calcPc = false;
    return retVal;
}
