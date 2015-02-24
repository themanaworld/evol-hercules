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

#include "map/data/itemd.h"
#include "map/struct/itemdext.h"
#include "map/npc.h"

bool eitemdb_is_item_usable(struct item_data *item)
{
    hookStop();
    if (!item)
        return false;
    return item->type == IT_HEALING || item->type == IT_USABLE || item->type == IT_CASH || item->type == IT_PETEGG;
}

void eitemdb_readdb_additional_fields(int *itemid,
                                      config_setting_t *it,
                                      int *n __attribute__ ((unused)),
                                      const char *source __attribute__ ((unused)))
{
    struct item_data *item = itemdb->exists(*itemid);
    int i32 = 0;
    if (!item)
    {
        hookStop();
        return;
    }
    struct ItemdExt *data = itemd_get(item);
    if (!data)
    {
        hookStop();
        return;
    }

    config_setting_t *t = NULL;

    if (libconfig->setting_lookup_int(it, "FloorLifeTime", &i32) && i32 >= 0)
        data->floorLifeTime = i32;
    if ((t = libconfig->setting_get_member(it, "AllowPickup")))
        data->allowPickup = libconfig->setting_get_bool(t) ? 1 : 0;

    hookStop();
}
