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
#include "map/itemdb.h"
#include "map/map.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/script.h"
#include "map/status.h"

#include "emap/data/itemd.h"
#include "emap/data/npcd.h"
#include "emap/struct/itemdext.h"
#include "emap/struct/npcdext.h"

int class_move_speed[CLASS_COUNT];

void status_init(void)
{
    int f;
    for (f = 0; f < CLASS_COUNT; f ++)
        class_move_speed[f] = 150;
}

void estatus_set_viewdata_post(struct block_list *bl,
                               int *class_ __attribute__ ((unused)))
{
    if (!bl)
        return;
    if (bl->type != BL_NPC)
        return;
    TBL_NPC *const npc = (TBL_NPC*)bl;
    struct NpcdExt *data = npcd_get(npc);
    if (data && data->init == false && npc->vd)
    {
        data->init = true;
        npc->vd->sex = 3;
        if (npc->subtype == SCRIPT)
        {
            if (npc->u.scr.script)
            {
                // here some magic to set npc local variable .id to bl.id
                const int num = reference_uid(script->add_str(".id"), 0);
                if (!npc->u.scr.script->local.vars)
                    npc->u.scr.script->local.vars = i64db_alloc(DB_OPT_RELEASE_DATA);
                i64db_iput(npc->u.scr.script->local.vars, num, npc->bl.id);
            }
        }
    }
}

void estatus_read_job_db_sub(int *idxPtr,
                             const char *name __attribute__ ((unused)),
                             config_setting_t *jdb)
{
    int i32 = 0;
    const int idx = *idxPtr;
    if (itemdb->lookup_const(jdb, "MoveSpeed", &i32))
        class_move_speed[idx] = i32;
}

int estatus_calc_pc__post(int retVal,
                          struct map_session_data *sd,
                          enum e_status_calc_opt *opt __attribute__ ((unused)))
{
    if (!sd)
        return retVal;

    if (!sd->state.permanent_speed)
    {
        const int idx = pc->class2idx(sd->status.class_);
        sd->base_status.speed = class_move_speed[idx];
    }
    return retVal;
}

int estatus_calc_pc_additional(struct map_session_data* sd,
                               enum e_status_calc_opt *opt __attribute__ ((unused)))
{
    int f;
    int k;

    hookStop();

    for (f = 0; f < MAX_INVENTORY; f ++)
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

        // here can be refine bonuses
    }
    return 0;
}
