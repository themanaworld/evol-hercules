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
#include "../../../common/timer.h"
#include "../../../map/battle.h"
#include "../../../map/itemdb.h"
#include "../../../map/map.h"

#include "map/data/itemd.h"
#include "map/struct/itemdext.h"

int emap_addflooritem_post(int retVal,
                           struct item *item,
                           int *amount __attribute__ ((unused)),
                           int16 *m __attribute__ ((unused)),
                           int16 *x __attribute__ ((unused)),
                           int16 *y __attribute__ ((unused)),
                           int *first_charid __attribute__ ((unused)),
                           int *second_charid __attribute__ ((unused)),
                           int *third_charid __attribute__ ((unused)),
                           int *flags __attribute__ ((unused)))
{
    struct flooritem_data* fitem = (struct flooritem_data*)idb_get(map->id_db, retVal);
    if (fitem->cleartimer != INVALID_TIMER)
    {
        int timeout = battle->bc->flooritem_lifetime;
        struct ItemdExt *data = itemd_get_by_item(item);
        if (data)
            timeout = data->floorLifeTime;
        timer->delete(fitem->cleartimer, map->clearflooritem_timer);
        fitem->cleartimer = timer->add(timer->gettick() + timeout, map->clearflooritem_timer, fitem->bl.id, 0);
    }
    return retVal;
}
