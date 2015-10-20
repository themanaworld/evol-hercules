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
#include "map/itemdb.h"
#include "map/map.h"
#include "map/npc.h"
#include "map/pc.h"

#include "emap/data/itemd.h"
#include "emap/struct/itemdext.h"
#include "emap/npc.h"

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
                                      const char *source)
{
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

    config_setting_t *t = NULL;

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

    config_setting_t *group = libconfig->setting_get_member(it, "AllowCards");
    if (group)
    {
        int idx = 0;
        config_setting_t *it2 = NULL;
        int cnt = 0;
        while ((it2 = libconfig->setting_get_elem(group, idx ++)))
        {
            const char *name = config_setting_name(it2);
            if (name && strncmp(name, "id", 2) && strncmp(name, "Id", 2))
                continue;
            const int val = libconfig->setting_get_int(it2);
            if (name)
            {
                data->allowedCards[cnt].id = atoi(name + 2);
                data->allowedCards[cnt].amount = val;
            }
            else
            {
                data->allowedCards[cnt].id = val;
                data->allowedCards[cnt].amount = 1;
            }
            cnt ++;
        }
    }

    hookStop();
}

void edestroy_item_data(struct item_data* self, int *free_selfPtr)
{
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
