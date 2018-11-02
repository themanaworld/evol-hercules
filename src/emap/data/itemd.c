// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/battle.h"
#include "map/itemdb.h"

#include "emap/data/itemd.h"
#include "emap/struct/itemdext.h"

struct ItemdExt *itemd_get_by_item(const struct item *item)
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
    data->identified = true;
    data->requiredStr = 0;
    data->requiredAgi = 0;
    data->requiredVit = 0;
    data->requiredInt = 0;
    data->requiredDex = 0;
    data->requiredLuk = 0;
    data->requiredMaxHp = 0;
    data->requiredMaxSp = 0;
    data->requiredAtk = 0;
    data->requiredMAtkMin = 0;
    data->requiredMAtkMax = 0;
    data->requiredDef = 0;
    data->requiredMDef = 0;
    data->requiredSkill = 0;
    data->useEffect = -1;
    data->useFailEffect = -1;
    data->unequipEffect = -1;
    data->unequipFailEffect = -1;
    data->charmItem = false;
    data->dropScript = NULL;
    data->takeScript = NULL;
    data->insertScript = NULL;
    VECTOR_INIT(data->allowedCards);
    VECTOR_INIT(data->allowedAmmo);
    data->subX = 8;
    data->subY = 8;
    data->minRange = 0;
    data->tmpUseType = 0;
    return data;
}
