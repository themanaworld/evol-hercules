// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/conf.h"
#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/battle.h"
#include "map/itemdb.h"
#include "map/mob.h"

#include "emap/mob.h"

#include "emap/data/mobd.h"
#include "emap/struct/mobdext.h"

int emob_deleteslave_sub(struct block_list *bl, va_list ap)
{
    if (!bl)
    {
        hookStop();
        return 0;
    }
    TBL_MOB *md = (TBL_MOB *)bl;
    if (!md)
    {
        hookStop();
        return 0;
    }

    const int id = va_arg(ap, int);
    if (md->master_id > 0 && md->master_id == id)
    {
        if (md->db->status.mode & MD_SURVIVE_WITHOUT_MASTER)
        {
            md->master_id = 0;
            md->master_dist = 0;
        }
        else
        {
            status_kill(bl);
        }
    }

    hookStop();
    return 0;
}

void emob_read_db_additional_fields(struct mob_db *entry,
                                    struct config_setting_t *it,
                                    int *nPtr __attribute__ ((unused)),
                                    const char *source  __attribute__ ((unused)))
{
    int i32 = 0;

    struct MobdExt *data = mobd_get(entry);
    if (!data)
    {
        hookStop();
        return;
    }

    if (mob->lookup_const(it, "WalkMask", &i32))
        data->walkMask = i32;
}

int emob_read_db_mode_sub_post(int retVal,
                               struct mob_db *entry  __attribute__ ((unused)),
                               struct config_setting_t *t)
{
    struct config_setting_t *t2;

    if ((t2 = libconfig->setting_get_member(t, "SurviveWithoutMaster")))
        retVal |= libconfig->setting_get_bool(t2) ? MD_SURVIVE_WITHOUT_MASTER : 0;

    return retVal;
}
