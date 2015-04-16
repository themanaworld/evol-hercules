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
#include "../../../map/map.h"
#include "../../../map/npc.h"
#include "../../../map/pc.h"
#include "../../../map/status.h"

#include "map/data/npcd.h"
#include "map/struct/npcdext.h"

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

int estatus_calc_pc_(int retVal,
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
