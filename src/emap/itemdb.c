// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/conf.h"
#include "common/HPMi.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/itemdb.h"
#include "map/map.h"
#include "map/npc.h"
#include "map/pc.h"

#include "plugins/HPMHooking.h"

#include "emap/data/itemd.h"
#include "emap/struct/itemdext.h"
#include "emap/npc.h"

bool eitemdb_is_item_usable_pre(struct item_data **itemPtr)
{
    struct item_data *item = *itemPtr;
    hookStop();
    if (!item)
        return false;
    return item->type == IT_HEALING || item->type == IT_USABLE || item->type == IT_CASH || item->type == IT_PETEGG;
}

void eitemdb_readdb_additional_fields_pre(int *itemid,
                                          struct config_setting_t **itPtr,
                                          int *n __attribute__ ((unused)),
                                          const char **sourcePtr)
{
    struct config_setting_t *it = *itPtr;
    const char *source = *sourcePtr;
    struct item_data *item = itemdb->exists(*itemid);
    int i32 = 0;
    const char *str = NULL;

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

    struct config_setting_t *t = NULL;

    if (libconfig->setting_lookup_int(it, "FloorLifeTime", &i32) && i32 >= 0)
        data->floorLifeTime = i32;
    if ((t = libconfig->setting_get_member(it, "AllowPickup")))
        data->allowPickup = libconfig->setting_get_bool(t) ? 1 : 0;
    if (libconfig->setting_lookup_int(it, "RequiredStr", &i32) && i32 >= 0)
        data->requiredStr = i32;
    if (libconfig->setting_lookup_int(it, "RequiredAgi", &i32) && i32 >= 0)
        data->requiredAgi = i32;
    if (libconfig->setting_lookup_int(it, "RequiredVit", &i32) && i32 >= 0)
        data->requiredVit = i32;
    if (libconfig->setting_lookup_int(it, "RequiredInt", &i32) && i32 >= 0)
        data->requiredInt = i32;
    if (libconfig->setting_lookup_int(it, "RequiredDex", &i32) && i32 >= 0)
        data->requiredDex = i32;
    if (libconfig->setting_lookup_int(it, "RequiredLuk", &i32) && i32 >= 0)
        data->requiredLuk = i32;
    if (libconfig->setting_lookup_int(it, "RequiredMaxHp", &i32) && i32 >= 0)
        data->requiredMaxHp = i32;
    if (libconfig->setting_lookup_int(it, "RequiredMaxSp", &i32) && i32 >= 0)
        data->requiredMaxSp = i32;
    if (libconfig->setting_lookup_int(it, "RequiredAtk", &i32) && i32 >= 0)
        data->requiredAtk = i32;
    if (libconfig->setting_lookup_int(it, "RequiredMAtkMin", &i32) && i32 >= 0)
        data->requiredMAtkMin = i32;
    if (libconfig->setting_lookup_int(it, "RequiredMAtkMax", &i32) && i32 >= 0)
        data->requiredMAtkMax = i32;
    if (libconfig->setting_lookup_int(it, "RequiredDef", &i32) && i32 >= 0)
        data->requiredDef = i32;
    if (libconfig->setting_lookup_int(it, "RequiredMDef", &i32) && i32 >= 0)
        data->requiredMDef = i32;
    if (itemdb->lookup_const(it, "RequiredSkill", &i32))
        data->requiredSkill = i32;
    if (libconfig->setting_lookup_bool(it, "Charm", &i32) && i32 >= 0)
        data->charmItem = i32 ? true : false;
    if ((t = libconfig->setting_get_member(it, "MaxFloorOffset")))
    {
        if (config_setting_is_aggregate(t))
        {
            data->subX = libconfig->setting_get_int_elem(t, 0);
            if (libconfig->setting_length(t) >= 2)
                data->subY = libconfig->setting_get_int_elem(t, 1);
            else if (libconfig->setting_length(t) >= 1)
                data->subY = data->subX;
        }
        else if (libconfig->setting_lookup_int(it, "MaxFloorOffset", &i32))
        {
            data->subX = i32;
            data->subY = i32;
        }
        if (data->subX < 0 || data->subX > 127)
        {
            ShowWarning("Field MaxFloorOffsetX for item %s must be in range 0-127\n", item->name);
            data->subX = 8;
        }
        if (data->subY < 0 || data->subY > 127)
        {
            ShowWarning("Field MaxFloorOffsetY for item %s must be in range 0-127\n", item->name);
            data->subY = 8;
        }
    }

    if (itemdb->lookup_const(it, "UseEffect", &i32))
        data->useEffect = i32;
    if (itemdb->lookup_const(it, "UseFailEffect", &i32))
        data->useFailEffect = i32;
    if (itemdb->lookup_const(it, "UnequipEffect", &i32))
        data->unequipEffect = i32;
    if (itemdb->lookup_const(it, "UnequipFailEffect", &i32))
        data->unequipFailEffect = i32;

    if (libconfig->setting_lookup_string(it, "OnDropScript", &str))
        data->dropScript = *str ? script->parse(str, source, -item->nameid, SCRIPT_IGNORE_EXTERNAL_BRACKETS, NULL) : NULL;
    if (libconfig->setting_lookup_string(it, "OnTakeScript", &str))
        data->takeScript = *str ? script->parse(str, source, -item->nameid, SCRIPT_IGNORE_EXTERNAL_BRACKETS, NULL) : NULL;
    if (libconfig->setting_lookup_string(it, "OnInsertCardScript", &str))
        data->insertScript = *str ? script->parse(str, source, -item->nameid, SCRIPT_IGNORE_EXTERNAL_BRACKETS, NULL) : NULL;

    struct config_setting_t *group = libconfig->setting_get_member(it, "AllowCards");
    if (group)
    {
        int idx = 0;
        struct config_setting_t *it2 = NULL;
        int cnt = VECTOR_LENGTH(data->allowedCards);
        while ((it2 = libconfig->setting_get_elem(group, idx ++)))
        {
            const char *name = config_setting_name(it2);
            if (name && strncmp(name, "id", 2) && strncmp(name, "Id", 2))
                continue;
            const int val = libconfig->setting_get_int(it2);

            VECTOR_ENSURE(data->allowedCards, cnt + 1, 1);
            VECTOR_INSERTZEROED(data->allowedCards, cnt);
            struct ItemCardExt *allowedCard = &VECTOR_INDEX(data->allowedCards, cnt);

            if (name)
            {
                allowedCard->id = atoi(name + 2);
                allowedCard->amount = val;
            }
            else
            {
                allowedCard->id = val;
                allowedCard->amount = 1;
            }
            cnt ++;
        }
    }

    group = libconfig->setting_get_member(it, "AllowAmmo");
    if (group)
    {
        int idx = 0;
        struct config_setting_t *it2 = NULL;
        int cnt = VECTOR_LENGTH(data->allowedAmmo);
        while ((it2 = libconfig->setting_get_elem(group, idx ++)))
        {
            const char *name = config_setting_name(it2);
            if (name && strncmp(name, "id", 2) && strncmp(name, "Id", 2))
                continue;
            VECTOR_ENSURE(data->allowedAmmo, cnt + 1, 1);
            VECTOR_PUSH(data->allowedAmmo, atoi(name + 2));
            cnt ++;
        }
    }

    hookStop();
}

void edestroy_item_data_pre(struct item_data **selfPtr,
                            int *free_selfPtr __attribute__ ((unused)))
{
    struct item_data *self = *selfPtr;
    struct ItemdExt *data = itemd_get(self);
    if (!data)
        return;

    if (data->dropScript)
        script->free_code(data->dropScript);
    if (data->takeScript)
        script->free_code(data->takeScript);
    if (data->insertScript)
        script->free_code(data->insertScript);
}
