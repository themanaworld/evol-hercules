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
#include "emap/lang.h"

struct DBMap *craftvar_db = NULL;
struct DBMap *craftconf_db = NULL;

int craft_counter = 0;

struct craft_db_entry *craft_create_db_entry(const int id)
{
    struct craft_db_entry *entry = aCalloc(sizeof(struct craft_db_entry), 1);
    if (!entry)
        return NULL;
    VECTOR_INIT(entry->inventories);
    VECTOR_INIT(entry->create_items);
    VECTOR_INIT(entry->delete_items);
    VECTOR_INIT(entry->required_items);
    VECTOR_INIT(entry->required_equips);
    VECTOR_INIT(entry->required_skills);
    VECTOR_INIT(entry->required_quests);
    return entry;
}

static bool craft_lookup_const(const config_setting_t *it, const char *name, int *value)
{
    if (libconfig->setting_lookup_int(it, name, value))
    {
        return true;
    }
    else
    {
        const char *str = NULL;
        if (libconfig->setting_lookup_string(it, name, &str))
        {
            if (*str && script->get_constant(str, value))
                return true;
        }
    }
    return false;
}

static bool craft_get_const(const config_setting_t *it, int *value)
{
    const char *str = libconfig->setting_get_string(it);
    if (str && *str && script->get_constant(str, value))
        return true;

    *value = libconfig->setting_get_int(it);
    return true;
}

static int craft_get_item_id(struct craft_db_entry *entry,
                             const char *const errorMessage,
                             const char *const name,
                             const char *const fieldName)
{
    if (!strcmp(name, "Empty"))
        return 0;

    struct item_data* id = itemdb->search_name(name);
    if (!id)
    {
        ShowWarning(errorMessage, entry->id, fieldName, name);
        return 0;
    }
    return id->nameid;
}

static void craft_read_source_inventory(struct craft_db_entry *entry,
                                        config_setting_t *tt)
{
    int i32;
    int i = 0;
    if (!tt || !config_setting_is_group(tt))
        return;

    config_setting_t *item;

    int invLen = VECTOR_LENGTH(entry->inventories);
    VECTOR_ENSURE(entry->inventories, invLen + 1, 1);
    VECTOR_INSERTZEROED(entry->inventories, invLen);
    struct craft_db_inventory *inventory = &VECTOR_INDEX(entry->inventories, invLen);

    while((item = libconfig->setting_get_elem(tt, i)) && i < craft_inventory_size)
    {
        int amount = 0;
        const char *name = config_setting_name(item);
        int itemId = craft_get_item_id(entry, "Wrong item name in craft %d in field %s in: %s\n", name, "SourceItems");
        if (!itemId)
        {
            inventory->items[i].index = 0;
            inventory->items[i].amount = 0;
            i ++;
            continue;
        }
        if (craft_get_const(item, &i32) && i32 >= 0)
            amount = i32;

        if (amount < 1)
        {
            inventory->items[i].index = 0;
            inventory->items[i].amount = 0;
            ShowWarning("Wrong item amount in craft %d in field SourceItems in: %d\n", entry->id, amount);
            i ++;
            continue;
        }

        inventory->items[i].index = itemId;
        inventory->items[i].amount = amount;
        i ++;
    }
}

static void craft_read_items_collection(struct craft_db_entry *entry,
                                        struct craft_items_collection *vector,
                                        config_setting_t *t,
                                        const char *const fieldName,
                                        enum craft_field_type fieldType)
{
    int i32;
    int i = 0;
    if (!t)
        return;

    config_setting_t *tt = libconfig->setting_get_member(t, fieldName);

    if (!tt || !config_setting_is_group(tt))
        return;

    config_setting_t *item;

    int len = 0;
    while((item = libconfig->setting_get_elem(tt, i)))
    {
        int amount = 0;
        const char *name = config_setting_name(item);
        int itemId = 0;
        if (fieldType == CRAFT_ITEM)
        {
            itemId = craft_get_item_id(entry, "Wrong item name in craft %d in field %s in: %s\n", name, fieldName);
            if (!itemId)
            {
                i ++;
                continue;
            }
        }
        else if (fieldType == CRAFT_QUEST)
        {
            if (!script->get_constant(name, &itemId))
            {
                ShowWarning("Wrong quest name in craft %d in field %s in: %s\n", entry->id, fieldName, name);
                i ++;
                continue;
            }
            if (!quest->db(itemId))
            {
                ShowWarning("Wrong quest id in craft %d in field %s in: %s\n", entry->id, fieldName, name);
                i ++;
                continue;
            }
        }
        else if (fieldType == CRAFT_SKILL)
        {
            itemId = skill->name2id(name);
            if (!itemId)
            {
                ShowWarning("Wrong skill name in craft %d in field %s in: %s\n", entry->id, fieldName, name);
                i ++;
                continue;
            }
        }
        else if (fieldType == CRAFT_BOOL)
        {
            itemId = craft_get_item_id(entry, "Wrong item name in craft %d in field %s in: %s\n", name, fieldName);
            if (!itemId)
            {
                i ++;
                continue;
            }
        }

        if (fieldType == CRAFT_BOOL)
        {
            if (!libconfig->setting_get_bool(item))
            {
                i ++;
                continue;
            }
            amount = 1;
        }
        else if (craft_get_const(item, &i32) && i32 >= 0)
        {
            amount = i32;
        }

        if (amount < 1)
        {
            if (fieldType == CRAFT_ITEM)
            {
                ShowWarning("Wrong item amount in craft %d in field %s in: %d\n", entry->id, fieldName, amount);
            }
            else if (fieldType == CRAFT_QUEST)
            {
                ShowWarning("Wrong quest level in craft %d in field %s in: %d\n", entry->id, fieldName, amount);
            }
            else if (fieldType == CRAFT_SKILL)
            {
                ShowWarning("Wrong skill level in craft %d in field %s in: %d\n", entry->id, fieldName, amount);
            }
            i ++;
            continue;
        }

        VECTOR_ENSURE(*vector, len + 1, 1);
        VECTOR_INSERTZEROED(*vector, len);
        struct item_pair *pair = &VECTOR_INDEX(*vector, len);
        len ++;

        pair->index = itemId;
        pair->amount = amount;
        //ShowInfo("%s: itemId=%d:%d\n", fieldName, itemId, amount);
        i ++;
    }
}

#define readField(name, var, def) \
    if (craft_lookup_const(craftt, name, &i32) && i32 >= 0) \
    { \
        entry->var = i32; \
    } \
    else \
    { \
        entry->var = def; \
    }

static bool craft_read_db_sub(config_setting_t *craftt, int id, const char *source)
{
    int class_;
    int i32;
    const char *str = NULL;
    config_setting_t *t;

    if (!libconfig->setting_lookup_int(craftt, "Id", &class_))
    {
        ShowWarning("craft_read_db_sub: Missing id in \"%s\", entry #%d, skipping.\n", source, class_);
        return false;
    }
    if (!libconfig->setting_lookup_string(craftt, "Name", &str) || !*str)
    {
        ShowWarning("craft_read_db_sub: Missing Name in craft %d of \"%s\", skipping.\n", class_, source);
        return false;
    }

    struct craft_db_entry *entry = craft_create_db_entry(class_);
    idb_put(craftconf_db, class_, entry);

    safestrncpy(entry->name, str, sizeof(entry->name));

    //ShowInfo("Craft: id=%d, name=%s\n", class_, entry->name);

    readField("Priority", priority, 0);
    readField("Price", price, 0);
    readField("Lv", level, 0);
    readField("Flag", flag, -1);

    if ((t = libconfig->setting_get_member(craftt, "SourceItems")) && config_setting_is_list(t))
    {
        int i, len = libconfig->setting_length(t);

        for (i = 0; i < len; i++)
        {
            craft_read_source_inventory(entry, libconfig->setting_get_elem(t, i));
        }
    }

    craft_read_items_collection(entry, &entry->create_items, craftt, "CreateItems", CRAFT_ITEM);
    craft_read_items_collection(entry, &entry->delete_items, craftt, "DeleteItems", CRAFT_ITEM);
    craft_read_items_collection(entry, &entry->required_items, craftt, "RequiredItems", CRAFT_ITEM);
    craft_read_items_collection(entry, &entry->required_skills, craftt, "RequiredSkills", CRAFT_ITEM);
    craft_read_items_collection(entry, &entry->required_quests, craftt, "RequiredQuests", CRAFT_QUEST);
    craft_read_items_collection(entry, &entry->required_equips, craftt, "RequiredEquips", CRAFT_BOOL);

    return true;
}
#undef readField

static void load_craft_db(const char *filename)
{
    config_t craft_db_conf;
    char filepath[256];
    config_setting_t *cdb;
    config_setting_t *t;
    int i = 0;

    nullpo_retv(filename);
    sprintf(filepath, "%s/%s", map->db_path, filename);

    if (libconfig->read_file(&craft_db_conf, filepath) ||
        !(cdb = libconfig->setting_get_member(craft_db_conf.root, "craft_db")))
    {
        ShowError("can't read %s\n", filepath);
        return;
    }

    while ((t = libconfig->setting_get_elem(cdb, i++)))
    {
        craft_read_db_sub(t, i - 1, filepath);
    }
    libconfig->destroy(&craft_db_conf);
    ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", i, filepath);

}

void do_init_craft(void)
{
    craftvar_db = idb_alloc(DB_OPT_RELEASE_BOTH);
    craftconf_db = idb_alloc(DB_OPT_RELEASE_BOTH);
    load_craft_db("craft_db.conf");
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

static void delete_craft_entry(struct craft_db_entry *entry)
{
    if (!entry)
        return;
    VECTOR_CLEAR(entry->inventories);
    VECTOR_CLEAR(entry->create_items);
    VECTOR_CLEAR(entry->delete_items);
    VECTOR_CLEAR(entry->required_items);
    VECTOR_CLEAR(entry->required_equips);
    VECTOR_CLEAR(entry->required_skills);
    VECTOR_CLEAR(entry->required_quests);
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

static int delete_craftconf_sub(DBKey key __attribute__ ((unused)),
                                DBData *data,
                                va_list args __attribute__ ((unused)))
{
    struct craft_db_entry *craft = DB->data2ptr(data);
    if (!craft)
        return 0;

    delete_craft_entry(craft);
    return 0;
}

void do_final_craft(void)
{
    craftvar_db->destroy(craftvar_db, delete_craftvar_sub);
    craftconf_db->destroy(craftconf_db, delete_craftconf_sub);
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
