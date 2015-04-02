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
#include "../../../map/battle.h"
#include "../../../map/itemdb.h"

#include "map/data/itemd.h"
#include "map/struct/itemdext.h"

struct ItemdExt *itemd_get_by_item(struct item *item)
{
    if (!item)
        return NULL;
    const int nameid = item->nameid;
    struct item_data *item_data = itemdb->exists(nameid);
    return itemd_get(item_data);
}

struct ItemdExt *itemd_get(struct item_data *item)
{
    if (!item)
        return NULL;

    struct ItemdExt *data = getFromITEMDATA(item, 0);
    if (!data)
    {
        data = itemd_create();
        addToITEMDATA(item, data, 0, true);
    }
    return data;
}

struct ItemdExt *itemd_create(void)
{
    struct ItemdExt *data = NULL;
    CREATE(data, struct ItemdExt, 1);
    if (!data)
        return NULL;
    data->floorLifeTime = battle->bc->flooritem_lifetime;
    data->allowPickup = true;
    data->requiredStr = 0;
    data->requiredAgi = 0;
    data->requiredVit = 0;
    data->requiredInt = 0;
    data->requiredDex = 0;
    data->requiredLuk = 0;
    return data;
}
