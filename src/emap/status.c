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
#include "common/utils.h"
#include "map/itemdb.h"
#include "map/map.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/script.h"
#include "map/status.h"

#include "plugins/HPMHooking.h"

#include "emap/npc.h"

#include "emap/effects.h"
#include "emap/horse.h"
#include "emap/skill_const.h"
#include "emap/status.h"
#include "emap/data/itemd.h"
#include "emap/data/npcd.h"
#include "emap/struct/itemdext.h"
#include "emap/struct/npcdext.h"

#include "emap/enum/esctype.h"
#include "emap/enum/esitype.h"

int class_move_speed[CLASS_COUNT];

void eInitChangeTables(void)
{
    status->set_sc(EVOL_PHYSICAL_SHIELD,
        (sc_type)SC_PHYSICAL_SHIELD,
        SCB_DEF | SCB_DEF2 | SCB_ASPD);
    status->dbs->IconChangeTable[EVOL_PHYSICAL_SHIELD].relevant_bl_types |= BL_SCEFFECT;
}

int estatus_init_post(int retVal,
                      bool minimal __attribute__ ((unused)))
{
    int f;
    for (f = 0; f < CLASS_COUNT; f ++)
        class_move_speed[f] = 150;

    eInitChangeTables();
    return retVal;
}

void estatus_set_viewdata_post(struct block_list *bl,
                               int class_ __attribute__ ((unused)))
{
    if (!bl)
        return;
    if (bl->type != BL_NPC)
        return;
    TBL_NPC *const npc = (TBL_NPC*)bl;
    struct NpcdExt *data = npcd_get(npc);
    if (data && data->init == false)
    {
        data->init = true;
        npc->vd.sex = 3;
        if (npc->subtype == SCRIPT)
        {
            if (npc->u.scr.script)
                enpc_set_var_num(npc, ".id", npc->bl.id);
        }
    }
}

void estatus_read_job_db_sub_post(int idx,
                                  const char *name __attribute__ ((unused)),
                                  struct config_setting_t *jdb)
{
    int i32 = 0;
    if (itemdb->lookup_const(jdb, "MoveSpeed", &i32))
        class_move_speed[idx] = i32;
}

int estatus_calc_pc__post(int retVal,
                          struct map_session_data *sd,
                          enum e_status_calc_opt opt __attribute__ ((unused)))
{
    if (!sd)
        return retVal;

    if (!sd->state.permanent_speed)
    {
        const int idx = pc->class2idx(sd->status.class);
        sd->base_status.speed = class_move_speed[idx];
    }
    return retVal;
}

void estatus_calc_pc_additional_pre(struct map_session_data **sdPtr,
                                    enum e_status_calc_opt *optPtr __attribute__ ((unused)))
{
    int f;
    int k;
    struct map_session_data *sd = *sdPtr;

    if (!sd)
    {
        hookStop();
        return;
    }

    for (f = 0; f < sd->status.inventorySize; f ++)
    {
        struct item_data *const item = sd->inventory_data[f];
        if (!item)
            continue;

        struct ItemdExt *data = itemd_get(item);
        if (!data || !data->charmItem)
            continue;

        for (k = 0; k < map->list[sd->bl.m].zone->disabled_items_count; k ++)
        {
            if (map->list[sd->bl.m].zone->disabled_items[k] == item->nameid)
                break;
        }

        if (k < map->list[sd->bl.m].zone->disabled_items_count)
            continue;

        if (!pc->isequip(sd, f))
            continue;

        struct status_data *bstatus = &sd->base_status;
        bstatus->def += item->def;

        script->run_use_script(sd, item, 0);

        // +++ TODO add item options bonuses too (for charm items)
        // here can be refine bonuses
    }

    horse_add_bonus(sd);

    hookStop();
}

unsigned short estatus_calc_speed_post(unsigned short retVal,
                                       struct block_list *bl,
                                       struct status_change *sc __attribute__ ((unused)),
                                       int speed __attribute__ ((unused)))
{
    return horse_add_speed_bonus(BL_CAST(BL_PC, bl), retVal);
}

defType estatus_calc_def_post(defType retVal,
                              struct block_list *bl __attribute__ ((unused)),
                              struct status_change *sc,
                              int def __attribute__ ((unused)),
                              bool viewable __attribute__ ((unused)))
{
    if (!sc)
        return retVal;

    if (sc->data[SC_PHYSICAL_SHIELD])
        retVal += sc->data[SC_PHYSICAL_SHIELD]->val1;

    return (defType)cap_value(retVal, DEFTYPE_MIN, DEFTYPE_MAX);
}

short estatus_calc_fix_aspd_post(short retVal,
                                 struct block_list *bl __attribute__ ((unused)),
                                 struct status_change *sc,
                                 int aspd __attribute__ ((unused)))
{
    if (!sc)
        return retVal;

    if (sc->data[SC_PHYSICAL_SHIELD])
        retVal -= sc->data[SC_PHYSICAL_SHIELD]->val2;

    return (short)cap_value(retVal, 0, 2000);
}

int estatus_change_start_post(int retVal,
                              struct block_list *src __attribute__ ((unused)),
                              struct block_list *bl __attribute__ ((unused)),
                              enum sc_type type __attribute__ ((unused)),
                              int rate __attribute__ ((unused)),
                              int val1 __attribute__ ((unused)),
                              int val2 __attribute__ ((unused)),
                              int val3 __attribute__ ((unused)),
                              int val4 __attribute__ ((unused)),
                              int tick __attribute__ ((unused)),
                              int flag __attribute__ ((unused)))
{
    if (!retVal)
        return retVal;

/*
    switch ((esc_type)type)
    {
        default:
            break;
    }
*/
    return retVal;
}

int estatus_change_end__post(int retVal,
                             struct block_list* bl __attribute__ ((unused)),
                             enum sc_type type __attribute__ ((unused)),
                             int tid __attribute__ ((unused)),
                             const char* file __attribute__ ((unused)),
                             int line __attribute__ ((unused)))
{
    if (!retVal)
        return retVal;

/*
    switch ((esc_type)type)
    {
        default:
            break;
    }
*/
    return retVal;
}

void estatus_calc_pc_recover_hp_pre(struct map_session_data **sdPtr __attribute__ ((unused)),
                                    struct status_data **bstatusPtr)
{
    struct status_data *bstatus = *bstatusPtr;
    nullpo_retv(bstatus);

    bstatus->hp = APPLY_RATE(bstatus->max_hp, battle->bc->restart_hp_rate);
    hookStop();
}
