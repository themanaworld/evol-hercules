// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/battle.h"
#include "map/itemdb.h"
#include "map/mob.h"

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
        if (md->db->status.mode & 0x8000)
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
