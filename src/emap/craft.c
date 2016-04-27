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

static int delete_craftvar_sub(union DBKey key __attribute__ ((unused)),
                               struct DBData *data,
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
           sd->status.inventory[f].amount < amount ||
           sd->status.inventory[f].equip > 0)
        {
            return false;
        }
    }
    return true;
}

struct craft_vardata *craft_str_to_craft(const char *craftstr)
{
    if (!craftstr)
        return false;
    struct strutil_data *craftdata = strutil_split(craftstr, '|', craft_inventory_size + 1);
    if (!craftdata)
        return false;

    if (craftdata->len < 1)
    {
        strutil_free(craftdata);
        return NULL;
    }

    struct craft_vardata *vardata = aCalloc(1, sizeof(struct craft_vardata));
    if (!vardata)
        return false;
    vardata->entry_id = -1;
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
    if (!sd)
        return;
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
    if (!sd)
        return false;
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
                sd->status.inventory[invIndex].amount <= 0 ||
                sd->status.inventory[invIndex].equip > 0)
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
           sd->status.inventory[f].amount < amount ||
           sd->status.inventory[f].equip > 0)
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
    if (!sd)
        return -1;
    for (i = 0; i < MAX_INVENTORY; i++)
    {
        if (sd->status.inventory[i].nameid == id &&
            sd->status.inventory[i].amount >= amount &&
            sd->status.inventory[i].equip == 0)
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
    if (!sd)
        return -1;
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

static int find_local_inventory_item(struct item_pair *local_inventory,
                                     const int id)
{
    int i;
    if (!local_inventory)
        return -1;
    for (i = 0; i < MAX_INVENTORY; i++)
    {
        struct item_pair *pair = &local_inventory[i];
        if (pair->index == id &&
            pair->amount >= 0)
        {
            return i;
        }
    }
    return -1;
}

static bool check_items_collection(struct item_pair *local_inventory,
                                   struct craft_items_collection *vector)
{
    int len = VECTOR_LENGTH(*vector);
    int i;
    if (!local_inventory)
        return false;
    if (len > 0)
    {
        for (i = 0; i < len; i ++)
        {
            struct item_pair *itemPair = &VECTOR_INDEX(*vector, i);
            int needAmount = itemPair->amount;
            while (needAmount > 0)
            {
                const int index =  find_local_inventory_item(local_inventory,
                    itemPair->index);
                if (index < 0)
                    return false;
                struct item_pair *localPair = &local_inventory[index];
                if (needAmount > localPair->amount)
                {
                    needAmount -= localPair->amount;
                    localPair->index = 0;
                    localPair->amount = 0;
                }
                else
                {
                    localPair->amount -= needAmount;
                    needAmount = 0;
                    if (localPair->amount == 0)
                        localPair->index = 0;
                }
            }
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
    if (!sd)
        return false;
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
    if (!sd)
        return false;
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
                              struct item_pair *craft_inventory)
{
    if (!entry || !craft_inventory)
        return false;
    int inv_count = VECTOR_LENGTH(entry->inventories);
    bool correct = true;

    int f;
    for (f = 0; f < inv_count; f ++)
    {
        struct craft_db_inventory *entry_inventory = &VECTOR_INDEX(entry->inventories, f);
        int i;
        for (i = 0; i < craft_inventory_size; i ++)
        {
            struct item_pair *invItem = &craft_inventory[i];
            struct item_pair *entryItem = &entry_inventory->items[i];
            if (invItem->index != entryItem->index ||
                invItem->amount < entryItem->amount)
            {   // items not same or amount too small, skipping
                correct = false;
                continue;
            }
        }
        if (correct)
        {
            entry->selected_inventory = entry_inventory;
            return true;
        }
        correct = true;
    }
    return false;
}

static void simplify_craftvar(TBL_PC *sd,
                              struct item_pair *craft_inventory,
                              struct craft_vardata *craft)
{
    int i;

    if (!craft || !craft_inventory)
        return;
    // combine different slots from inventory var into one slot with id and amount
    for (i = 0; i < craft_inventory_size; i ++)
    {
        struct item_pair *invPair = &craft_inventory[i];
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

static void init_inventory_copy(TBL_PC *sd,
                                struct item_pair *local_inventory)
{
    int f;
    if (!sd || !local_inventory)
        return;
    for (f = 0; f < MAX_INVENTORY; f ++)
    {
        const int id = sd->status.inventory[f].nameid;
        const int amount = sd->status.inventory[f].amount;
        struct item_pair *pair = &local_inventory[f];
        if(id > 0 &&
           amount > 0 &&
           sd->status.inventory[f].equip == 0)
        {
            pair->index = id;
            pair->amount = amount;
        }
        else
        {
            pair->index = 0;
            pair->amount = 0;
        }
    }
}

static bool apply_craft_inventory(struct craft_db_inventory *entry_inventory,
                                  struct craft_vardata *craft,
                                  struct item_pair *local_inventory)
{
    int f;
    if (!entry_inventory || !craft || !local_inventory)
        return false;
    for (f = 0; f < craft_inventory_size; f ++)
    {
        struct item_pair *entryItem = &entry_inventory->items[f];
        int needAmount = entryItem->amount;
        struct craft_slot *craftSlot = &craft->slots[f];
        int i;
        int len = VECTOR_LENGTH(craftSlot->items);
        for (i = 0; i < len && needAmount; i ++)
        {
            struct item_pair *craftItem = &VECTOR_INDEX(craftSlot->items, i);
            if (needAmount > craftItem->amount)
            {
                local_inventory[craftItem->index].amount -= craftItem->amount;
                needAmount -= craftItem->amount;
            }
            else
            {
                local_inventory[craftItem->index].amount -= needAmount;
                needAmount = 0;
            }
            if (local_inventory[craftItem->index].amount == 0)
                local_inventory[craftItem->index].index = 0;
        }
        if (needAmount > 0)
            return false;
    }
    return true;
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
    struct item_pair local_inventory[MAX_INVENTORY];    // id, amount

    init_inventory_copy(sd, &local_inventory[0]);

    struct DBIterator* iter = db_iterator(craftconf_db);

    for (entry = dbi_first(iter); dbi_exists(iter); entry = dbi_next(iter))
    {
        //ShowInfo("check recipes: %d\n", entry->id);
        if ((flag && !(entry->flag & flag)) ||
            sd->status.zeny < entry->price ||
            sd->status.base_level < entry->level)
        {
            continue;
        }

        struct item_pair temp_inventory[MAX_INVENTORY];    // id, amount
        memcpy(&temp_inventory[0], &local_inventory[0], MAX_INVENTORY * sizeof(struct item_pair));

        //ShowInfo("base correct\n");
        if (!check_inventories(sd, entry, inventory))
            continue;
        //ShowInfo("inventories correct\n");
        if (!apply_craft_inventory(entry->selected_inventory, craft, &temp_inventory[0]))
            continue;
        //ShowInfo("apply craft correct\n");
        if (!check_items_collection(&temp_inventory[0], &entry->delete_items))
            continue;
        //ShowInfo("delete_items correct\n");
        if (!check_items_collection(&temp_inventory[0], &entry->required_items))
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

    struct item_pair craft_inventory[craft_inventory_size];
    simplify_craftvar(sd, &craft_inventory[0], craft);
    const int recipe = craft_get_recipe(sd, craft, &craft_inventory[0], flag);
    craft->entry_id = recipe;
    //ShowInfo("found recipe: %d\n", recipe);
    return recipe;
}

static bool craft_delete_items(TBL_PC *sd,
                               struct craft_items_collection *vector)
{
    if (!sd || !vector)
        return false;
    int len = VECTOR_LENGTH(*vector);
    int i;
    if (len > 0)
    {
        for (i = 0; i < len; i ++)
        {
            struct item_pair *itemPair = &VECTOR_INDEX(*vector, i);
            const int index = find_inventory_item(sd, itemPair->index, itemPair->amount);
            if (!index)
                return false;
            pc->delitem(sd, index, itemPair->amount, 2, 8, LOG_TYPE_PRODUCE);
        }
    }
    return true;
}

static bool craft_create_items(TBL_PC *sd,
                               struct craft_db_entry *entry)
{
    if (!sd || !entry)
        return false;
    const int vars = VECTOR_LENGTH(entry->create_items);
    struct craft_items_collection *vector = &VECTOR_INDEX(entry->create_items,
        (rand() % (vars * 10)) / 10);

    int len = VECTOR_LENGTH(*vector);
    int i;
    if (len > 0)
    {
        struct item it;
        for (i = 0; i < len; i ++)
        {
            struct item_pair *itemPair = &VECTOR_INDEX(*vector, i);
            memset(&it, 0, sizeof(it));
            it.nameid = itemPair->index;
            it.amount = itemPair->amount;
            it.identify = 1;
            pc->additem(sd, &it, itemPair->amount, LOG_TYPE_PRODUCE);
        }
    }
    return true;
}

static bool craft_delete_inventory(TBL_PC *sd,
                                   struct craft_vardata *craft,
                                   struct craft_db_entry *entry)
{
    if (!sd || !craft || !entry)
        return false;

    struct craft_db_inventory *entry_inventory = entry->selected_inventory;
    if (!entry_inventory)
        return false;

    int f;
    for (f = 0; f < craft_inventory_size; f ++)
    {
        struct item_pair *entryItem = &entry_inventory->items[f];
        int needAmount = entryItem->amount;
        struct craft_slot *craftSlot = &craft->slots[f];
        int i;
        int len = VECTOR_LENGTH(craftSlot->items);
        for (i = 0; i < len && needAmount; i ++)
        {
            struct item_pair *craftItem = &VECTOR_INDEX(craftSlot->items, i);
            if (needAmount > craftItem->amount)
            {
                pc->delitem(sd, craftItem->index, craftItem->amount, 2, 8, LOG_TYPE_PRODUCE);
                needAmount -= craftItem->amount;
            }
            else
            {
                pc->delitem(sd, craftItem->index, needAmount, 2, 8, LOG_TYPE_PRODUCE);
                needAmount = 0;
            }
        }
        if (needAmount > 0)
        {
            ShowError("Craft broken amount. Probably exploit in use: '"
                CL_WHITE"%s"CL_RESET"'"
                " (AID/CID: '"CL_WHITE"%d/%d"CL_RESET"'.\n",
                sd->status.name, sd->status.account_id, sd->status.char_id);
            return false;
        }
    }

    return true;
}

bool craft_use(TBL_PC *sd,
               const int id)
{
    struct craft_vardata *craft = idb_get(craftvar_db, id);
    if (!sd)
        return false;
    if (!craft)
    {
        ShowError("Craft object with id %d not exists.\n", id);
        return false;
    }
    struct craft_db_entry *entry = idb_get(craftconf_db, craft->entry_id);
    if (!entry)
    {
        ShowError("Craft config entry with id %d not exists.\n", craft->entry_id);
        return false;
    }

    if (!craft_delete_inventory(sd, craft, entry))
        return false;

    if (!craft_delete_items(sd, &entry->delete_items))
        return false;

    if (sd->status.zeny < entry->price)
    {
        ShowError("Craft broken zeny. Probably exploit in use: '"
            CL_WHITE"%s"CL_RESET"'"
            " (AID/CID: '"CL_WHITE"%d/%d"CL_RESET"'.\n",
            sd->status.name, sd->status.account_id, sd->status.char_id);
        return false;
    }
    sd->status.zeny -= entry->price;

    craft_create_items(sd, entry);

    clif->updatestatus(sd, SP_ZENY);
    clif->updatestatus(sd, SP_WEIGHT);
    return true;
}

int craft_get_entry_code(TBL_PC *sd,
                         const int id)
{
    struct craft_db_entry *entry = idb_get(craftconf_db, id);
    if (!entry)
    {
        ShowError("Craft config entry with id %d not exists.\n", id);
        return -1;
    }
    return entry->return_code;
}
