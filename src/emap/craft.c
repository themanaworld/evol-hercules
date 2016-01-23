// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/itemdb.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/quest.h"

#include "ecommon/utils/strutil.h"

#include "ecommon/struct/strutildata.h"

#include "emap/craft.h"
#include "emap/craftconf.h"
#include "emap/lang.h"

struct DBMap *craftvar_db = NULL;

int craft_counter = 0;

void do_init_craft(void)
{
    craftvar_db = idb_alloc(DB_OPT_RELEASE_BOTH);
}

static void delete_craft_var(struct craft_vardata *craft)
{
    if (!craft)
        return;
    int index;
    for (index = 0; index < craft_inventory_size; index ++)
    {
        struct craft_slot *slot = &craft->slots[index];
        VECTOR_CLEAR(slot->items);
    }
}

static int delete_craftvar_sub(DBKey key __attribute__ ((unused)),
                               DBData *data,
                               va_list args __attribute__ ((unused)))
{
    struct craft_vardata *craft = DB->data2ptr(data);
    if (!craft)
        return 0;

    delete_craft_var(craft);
    return 0;
}

void do_final_craft(void)
{
    craftvar_db->destroy(craftvar_db, delete_craftvar_sub);
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
        int item_id = 0;
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
            const int new_item = sd->status.inventory[index].nameid;
            if (item_id != 0 && new_item != item_id)
            {   // different item id in same slot
                strutil_free(slotdata);
                strutil_free(craftdata);
                return false;
            }
            if (new_item != 0)
                item_id = new_item;
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
            if (index < 0 ||
                index >= MAX_INVENTORY)
            {   // wrong item index
                strutil_free(slotdata);
                strutil_free(craftdata);
                return false;
            }
            VECTOR_ENSURE(crslot->items, slot + 1, 1);
            VECTOR_INSERTZEROED(crslot->items, slot);
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

void craft_dump(TBL_PC *sd, const int id)
{
    struct craft_vardata *craft = idb_get(craftvar_db, id);
    if (!craft)
    {
        ShowError("Craft object with id %d not exists.\n", id);
        return;
    }
    int index;
    ShowInfo("craft dump: %d\n", id);
    for (index = 0; index < craft_inventory_size; index ++)
    {
        struct craft_slot *crslot = &craft->slots[index];
        const int len = VECTOR_LENGTH(crslot->items);
        int slot;
        if (len > 0)
            ShowInfo(" index: %d\n", index);
        for (slot = 0; slot < len; slot ++)
        {
            struct item_pair *pair = &VECTOR_INDEX(crslot->items, slot);
            const int invIndex = pair->index;
            if (invIndex >= 0 &&
                invIndex < MAX_INVENTORY &&
                sd->status.inventory[invIndex].nameid)
            {
                const int item_id = sd->status.inventory[invIndex].nameid;
                const struct item_data *i_data = itemdb->search(item_id);
                if (i_data)
                {
                    ShowInfo("  item: %d (%s,%d), %d\n",
                        invIndex,
                        lang_pctrans(i_data->jname, sd),
                        item_id,
                        pair->amount);
                }
                else
                {
                    ShowInfo("  item: %d, %d\n", invIndex, pair->amount);
                }
            }
            else
            {
                ShowInfo("  item: %d, %d\n", invIndex, pair->amount);
            }
        }
    }
}

void craft_delete(const int id)
{
    struct craft_vardata *craft = idb_get(craftvar_db, id);
    if (!craft)
    {
        ShowError("Craft object with id %d not exists.\n", id);
        return;
    }
    delete_craft_var(craft);
    idb_remove(craftvar_db, id);
}

struct craft_slot *craft_get_slot(const int id, const int slot)
{
    struct craft_vardata *craft = idb_get(craftvar_db, id);
    if (!craft)
    {
        ShowError("Craft object with id %d not exists.\n", id);
        return NULL;
    }
    if (slot < 0 || slot > craft_inventory_size)
    {
        ShowError("Wrong slot %d for craft with id %d.\n", slot, id);
        return NULL;
    }
    return &craft->slots[slot];
}

bool craft_validate(TBL_PC *sd, const int id)
{
    struct craft_vardata *craft = idb_get(craftvar_db, id);
    if (!craft)
    {
        ShowError("Craft object with id %d not exists.\n", id);
        return false;
    }
    int amounts[MAX_INVENTORY];
    int f;

    for (f = 0; f < MAX_INVENTORY; f ++)
        amounts[f] = 0;

    int index;
    for (index = 0; index < craft_inventory_size; index ++)
    {
        struct craft_slot *crslot = &craft->slots[index];
        const int len = VECTOR_LENGTH(crslot->items);
        int slot;
        for (slot = 0; slot < len; slot ++)
        {
            struct item_pair *pair = &VECTOR_INDEX(crslot->items, slot);
            const int invIndex = pair->index;
            if (invIndex < 0 ||
                invIndex >= MAX_INVENTORY ||
                !sd->status.inventory[invIndex].nameid ||
                !sd->status.inventory[invIndex].amount)
            {
                return false;
            }
            amounts[invIndex] += pair->amount;
        }
    }
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

static int find_inventory_item(TBL_PC *sd,
                               const int id,
                               const int amount)
{
    int i;
    for (i = 0; i < MAX_INVENTORY; i++)
    {
        if (sd->status.inventory[i].nameid == id &&
            sd->status.inventory[i].amount >= amount)
        {
            return i;
        }
    }
    return -1;
}

static int find_inventory_equipped_item(TBL_PC *sd,
                                        const int id)
{
    int i;
    for (i = 0; i < MAX_INVENTORY; i++)
    {
        if (sd->status.inventory[i].nameid == id &&
            sd->status.inventory[i].amount > 0 &&
            sd->status.inventory[i].equip > 0)
        {
            return i;
        }
    }
    return -1;
}

static bool check_items_collection(TBL_PC *sd,
                                   struct craft_items_collection *vector)
{
    int len = VECTOR_LENGTH(*vector);
    int i;
    if (len > 0)
    {
        for (i = 0; i < len; i ++)
        {
            struct item_pair *itemPair = &VECTOR_INDEX(*vector, i);
            if (find_inventory_item(sd, itemPair->index, itemPair->amount) < 0)
                return false;
        }
    }
    return true;
}

static bool check_equips(TBL_PC *sd,
                         struct craft_items_collection *vector)
{
    int len = VECTOR_LENGTH(*vector);
    int i;
    if (len > 0)
    {
        for (i = 0; i < len; i ++)
        {
            struct item_pair *itemPair = &VECTOR_INDEX(*vector, i);
            if (find_inventory_equipped_item(sd, itemPair->index) < 0)
                return false;
        }
    }
    return true;
}

static bool check_skills(TBL_PC *sd,
                         struct craft_items_collection *vector)
{
    int len = VECTOR_LENGTH(*vector);
    int i;
    if (len > 0)
    {
        for (i = 0; i < len; i ++)
        {
            struct item_pair *itemPair = &VECTOR_INDEX(*vector, i);
            const int index = skill->get_index(itemPair->index);
            if (!index)
                return false;
            if (sd->status.skill[index].lv < itemPair->amount)
                return false;
        }
    }
    return true;
}

static bool check_quests(TBL_PC *sd,
                         struct craft_items_collection *vector)
{
    int len = VECTOR_LENGTH(*vector);
    int i;
    if (len > 0)
    {
        for (i = 0; i < len; i ++)
        {
            struct item_pair *itemPair = &VECTOR_INDEX(*vector, i);

            int n;
            ARR_FIND(0, sd->avail_quests, n, sd->quest_log[n].quest_id == itemPair->index);
            if (n == sd->avail_quests)
                return false;
            if (sd->quest_log[n].count[0] < itemPair->amount)
                return false;
        }
    }
    return true;
}

static bool check_inventories(TBL_PC *sd,
                              struct craft_db_entry *entry,
                              struct item_pair *inventory)
{
    int inv_count = VECTOR_LENGTH(entry->inventories);
    bool correct = true;

    int f;
    for (f = 0; f < inv_count; f ++)
    {
        struct craft_db_inventory *entry_inventory = &VECTOR_INDEX(entry->inventories, f);
        int i;
        for (i = 0; i < craft_inventory_size; i ++)
        {
            struct item_pair *invItem = &inventory[i];
            struct item_pair *entryItem = &entry_inventory->items[i];
            if (invItem->index != entryItem->index ||
                invItem->amount < entryItem->amount)
            {   // items not same or amount too small, skipping
                correct = false;
                continue;
            }
        }
        if (correct)
            return true;
        correct = true;
    }
    return false;
}

static void simplify_craftvar(TBL_PC *sd,
                              struct item_pair *inventory,
                              struct craft_vardata *craft)
{
    int i;

    // combine different slots from inventory var into one slot with id and amount
    for (i = 0; i < craft_inventory_size; i ++)
    {
        struct item_pair *invPair = &inventory[i];
        invPair->index = 0;
        invPair->amount = 0;

        struct craft_slot *slot = &craft->slots[i];
        const int len = VECTOR_LENGTH(slot->items);
        int f;
        for (f = 0; f < len; f ++)
        {
            struct item_pair *pair = &VECTOR_INDEX(slot->items, f);
            const int itemIndex = pair->index;
            const int itemId = sd->status.inventory[itemIndex].nameid;
            // additional check for craft var
            if (invPair->index != 0 && invPair->index != itemId)
                continue;
            invPair->index = itemId;
            invPair->amount += sd->status.inventory[itemIndex].amount;
        }
    }
}

static int craft_get_recipe(TBL_PC *sd,
                            struct craft_vardata *craft,
                            struct item_pair *inventory,
                            const int flag)
{
    if (!sd || !craft || !inventory)
        return -1;

    struct craft_db_entry *best_entry = NULL;
    struct craft_db_entry *entry;

    DBIterator* iter = db_iterator(craftconf_db);

    for (entry = dbi_first(iter); dbi_exists(iter); entry = dbi_next(iter))
    {
        //ShowInfo("check recipes: %d\n", entry->id);
        if ((flag && !(entry->flag & flag)) ||
            sd->status.zeny < entry->price ||
            sd->status.base_level < entry->level)
        {
            continue;
        }
        //ShowInfo("base correct\n");
        if (!check_inventories(sd, entry, inventory))
            continue;
        //ShowInfo("inventories correct\n");
        if (!check_items_collection(sd, &entry->delete_items))
            continue;
        //ShowInfo("delete_items correct\n");
        if (!check_items_collection(sd, &entry->required_items))
            continue;
        //ShowInfo("required_items correct\n");
        if (!check_equips(sd, &entry->required_equips))
            continue;
        //ShowInfo("required_equips correct\n");
        if (!check_skills(sd, &entry->required_skills))
            continue;
        //ShowInfo("required_quests correct\n");
        if (!check_quests(sd, &entry->required_quests))
            continue;

        //ShowInfo("found\n");
        if (best_entry == NULL ||
            entry->priority > best_entry->priority ||
            entry->id < best_entry->id)
        {
            best_entry = entry;
        }
    }
    dbi_destroy(iter);
    return best_entry ? best_entry->id : -1;
}

int craft_find_entry(TBL_PC *sd,
                     const int craftvar,
                     const int flag)
{
    if (!sd)
        return -1;

    struct craft_vardata *craft = idb_get(craftvar_db, craftvar);
    if (!craft)
    {
        ShowError("Craft object with id %d not exists.\n", craftvar);
        return -1;
    }

    struct item_pair inventory[craft_inventory_size];
    simplify_craftvar(sd, &inventory[0], craft);
    const int recipe = craft_get_recipe(sd, craft, &inventory[0], flag);
    //ShowInfo("found recipe: %d\n", recipe);
    return recipe;
}
