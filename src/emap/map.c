// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/db.h"
#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/battle.h"
#include "map/itemdb.h"
#include "map/map.h"
#include "map/pc.h"

#include "emap/permission.h"
#include "emap/send.h"
#include "emap/data/itemd.h"
#include "emap/data/session.h"
#include "emap/struct/itemdext.h"
#include "emap/struct/sessionext.h"

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
    TBL_ITEM* fitem = (TBL_ITEM*)idb_get(map->id_db, retVal);
    if (fitem && fitem->cleartimer != INVALID_TIMER)
    {
        int timeout = battle->bc->flooritem_lifetime;
        struct ItemdExt *data = itemd_get_by_item(item);
        if (data)
            timeout = data->floorLifeTime;
        timer->delete(fitem->cleartimer, map->clearflooritem_timer);
        if (timeout >= 0)
            fitem->cleartimer = timer->add(timer->gettick() + timeout, map->clearflooritem_timer, fitem->bl.id, 0);
    }
    return retVal;
}

void emap_online_list(int fd)
{
    char *buf = aCalloc (1, 20000);
    char *ptr = buf;
    TBL_PC* sd;

    struct SessionExt *data1 = session_get(fd);
    if (!data1)
    {
        aFree(buf);
        return;
    }

    const time_t t = time(NULL);
    if (data1->onlinelistlasttime + 15 >= t)
    { // not more than 1 per 15 seconds
        data1->onlinelistlasttime = t;
        aFree(buf);
        return;
    }

    TBL_PC* ssd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!ssd)
    {
        aFree(buf);
        return;
    }

    const bool showVersion = pc_has_permission(ssd, permission_show_client_version_flag);
    const int gpoupLevel = pc_get_group_level(ssd);
    data1->onlinelistlasttime = t;

    DBIterator* iter = db_iterator(map->pc_db);

    for (sd = dbi_first(iter); dbi_exists(iter); sd = dbi_next(iter))
    {
        if (!sd)
            continue;

        if (ptr - buf > 19500)
            break;

        if (pc_isinvisible(sd) && gpoupLevel < pc_get_group_level(sd))
            continue;

        struct SessionExt *data = session_get_bysd(sd);
        if (!data)
            continue;

        uint8 state = data->state;
        if (sd->status.sex == 1)
            state |= 128;
        else
            state = (state | 128) ^ 128;

        if (pc_has_permission(sd, permission_send_gm_flag))
            state |= 64;
        else
            state = (state | 64) ^ 64;

        *ptr = state;
        ptr ++;

        *ptr = sd->status.base_level;
        ptr ++;

        if (showVersion)
            *ptr = data->clientVersion;
        else
            *ptr = 0;
        ptr ++;

        strcpy(ptr, sd->status.name);
        ptr += strlen(sd->status.name);
        *ptr = 0;
        ptr ++;

    }
    dbi_destroy(iter);
    send_online_list(fd, buf, ptr - buf);
    aFree(buf);
}