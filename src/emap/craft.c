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
#include "map/npc.h"
#include "map/pc.h"

#include "ecommon/utils/strutil.h"

#include "ecommon/struct/strutildata.h"

#include "emap/craft.h"

struct DBMap *craftvar_db = NULL;

int craft_counter = 0;

void do_init_craft(void)
{
    craftvar_db = idb_alloc(DB_OPT_RELEASE_BOTH);
}

static int delete_craft_sub(DBKey key __attribute__ ((unused)),
                            DBData *data,
                            va_list args __attribute__ ((unused)))
{
    struct craft_vardata *craft = DB->data2ptr(data);
    if (!craft)
        return 0;

    int index;
    for (index = 0; index < craft_inventory_size; index ++)
    {
        struct craft_slot *slot = &craft->slots[index];
        VECTOR_CLEAR(slot->items);
    }
    return 0;
}

void do_final_craft(void)
{
    craftvar_db->destroy(craftvar_db, delete_craft_sub);
}

bool craft_checkstr(TBL_PC *sd, const char *craftstr)
{
    if (!sd || !craftstr)
        return false;
    int amounts[MAX_INVENTORY];

    int f;
    for (f = 0; f < MAX_INVENTORY; f ++)
        amounts[f] = 0;

    struct strutil_data *craftdata = strutil_split(craftstr, '|', craft_inventory_size + 1);
    if (!craftdata)
        return false;

    if (craftdata->len < 1)
    {
        strutil_free(craftdata);
        return false;
    }
    int index;
    for (index = 0; index < craftdata->len; index ++)
    {
        const char *slotstr = VECTOR_INDEX(craftdata->parts, index + 1);
        if (!slotstr || !*slotstr)
            continue;
        struct strutil_data *slotdata = strutil_split(slotstr, ';', MAX_INVENTORY + 1);
        if (!slotdata)
        {
            strutil_free(slotdata);
            continue;
        }
        int slot;
        for (slot = 0; slot < slotdata->len; slot ++)
        {
            const char *itemstr = VECTOR_INDEX(slotdata->parts, slot + 1);
            if (!itemstr || !*itemstr)
                continue;
            int index;
            int amount;
            int cnt = sscanf(itemstr, "%d,%d", &index, &amount);
            if (cnt != 2)
            {
                index = atoi(itemstr);
                amount = 1;
            }
            if (index < 0 || index >= MAX_INVENTORY)
            {   // wrong item index
                strutil_free(slotdata);
                strutil_free(craftdata);
                return false;
            }
            amounts[index] += amount;
            if (amounts[index] > 32000)
            {   // slot overflow
                strutil_free(slotdata);
                strutil_free(craftdata);
                return false;
            }
        }
        strutil_free(slotdata);
    }
    strutil_free(craftdata);

    for (f = 0; f < MAX_INVENTORY; f ++)
    {
        const int amount = amounts[f];
        if (!amount)
            continue;
        if(sd->status.inventory[f].nameid == 0 ||
           sd->status.inventory[f].amount < amount)
        {
            return false;
        }
    }
    return true;
}

struct craft_vardata *craft_str_to_craft(const char *craftstr)
{
    struct strutil_data *craftdata = strutil_split(craftstr, '|', craft_inventory_size + 1);
    if (!craftdata)
        return false;

    if (craftdata->len < 1)
    {
        strutil_free(craftdata);
        return NULL;
    }

    struct craft_vardata *vardata = aCalloc(1, sizeof(struct craft_vardata));
    int index;
    for (index = 0; index < craftdata->len; index ++)
    {
        const char *slotstr = VECTOR_INDEX(craftdata->parts, index + 1);
        if (!slotstr || !*slotstr)
            continue;
        struct strutil_data *slotdata = strutil_split(slotstr, ';', MAX_INVENTORY + 1);
        if (!slotdata)
        {
            strutil_free(slotdata);
            continue;
        }
        struct craft_slot *crslot = &vardata->slots[index];
        VECTOR_INIT(crslot->items);
        int slot;
        for (slot = 0; slot < slotdata->len; slot ++)
        {
            const char *itemstr = VECTOR_INDEX(slotdata->parts, slot + 1);
            if (!itemstr || !*itemstr)
                continue;
            int index;
            int amount;
            int cnt = sscanf(itemstr, "%d,%d", &index, &amount);
            if (cnt != 2)
            {
                index = atoi(itemstr);
                amount = 1;
            }
            if (index < 0 || index >= MAX_INVENTORY)
            {   // wrong item index
                strutil_free(slotdata);
                strutil_free(craftdata);
                return false;
            }
            VECTOR_ENSURE(crslot->items, slot + 1, 1);
            struct item_pair *pair = &VECTOR_INDEX(crslot->items, slot);
            pair->index = index;
            pair->amount = amount;
        }
        strutil_free(slotdata);
    }
    strutil_free(craftdata);

    return vardata;
}

int str_to_craftvar(TBL_PC *sd, const char *craftstr)
{
    if (!craft_checkstr(sd, craftstr))
    {
        if (sd)
            ShowWarning("invalid craft: %d\n", sd->bl.id);
        return -1;
    }

    struct craft_vardata *craft = craft_str_to_craft(craftstr);
    if (!craft)
        return -1;
    craft_counter ++;
    idb_put(craftvar_db, craft_counter, craft);
    return craft_counter;
}
