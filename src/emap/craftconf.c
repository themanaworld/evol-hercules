// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/conf.h"
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

#include "emap/craftconf.h"

struct DBMap *craftconf_db = NULL;

struct craft_db_entry *craft_create_db_entry(const int id __attribute__ ((unused)))
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
    entry->selected_inventory = NULL;
//    entry->return_code = 0;
    return entry;
}

static bool craft_lookup_const(const struct config_setting_t *it, const char *name, int *value)
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

static bool craft_get_const(const struct config_setting_t *it, int *value)
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
    if (!strcmp(name, "Empty") || !entry || !errorMessage)
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
                                        struct config_setting_t *tt)
{
    int i32;
    int i = 0;
    if (!entry || !tt || !config_setting_is_group(tt))
        return;

    struct config_setting_t *item;

    int invLen = VECTOR_LENGTH(entry->inventories);
    VECTOR_ENSURE(entry->inventories, 1, 1);
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

static void craft_read_create_items(struct craft_db_entry *entry,
                                    struct config_setting_t *tt)
{
    int i32;
    int i = 0;
    if (!entry || !tt || !config_setting_is_group(tt))
        return;

    struct config_setting_t *item;

    int invLen = VECTOR_LENGTH(entry->create_items);
    VECTOR_ENSURE(entry->create_items, 1, 1);
    VECTOR_INSERTZEROED(entry->create_items, invLen);
    struct craft_items_collection *collection = &VECTOR_INDEX(entry->create_items, invLen);
    VECTOR_INIT(*collection);
    int collecitonLen = VECTOR_LENGTH(*collection);

    while((item = libconfig->setting_get_elem(tt, i)))
    {
        int amount = 0;
        const char *name = config_setting_name(item);
        int itemId = craft_get_item_id(entry, "Wrong item name in craft %d in field %s in: %s\n", name, "CreateItems");
        if (!itemId)
        {
            i ++;
            continue;
        }
        if (craft_get_const(item, &i32) && i32 >= 0)
            amount = i32;

        if (amount < 1)
        {
            ShowWarning("Wrong item amount in craft %d in field CreateItems in: %d\n", entry->id, amount);
            i ++;
            continue;
        }

        VECTOR_ENSURE(*collection, 1, 1);
        VECTOR_INSERTZEROED(*collection, collecitonLen);
        struct item_pair *pair = &VECTOR_INDEX(*collection, collecitonLen);
        pair->index = itemId;
        pair->amount = amount;

        collecitonLen ++;
        i ++;
    }
}

static void craft_read_items_collection(struct craft_db_entry *entry,
                                        struct craft_items_collection *vector,
                                        struct config_setting_t *t,
                                        const char *const fieldName,
                                        enum craft_field_type fieldType)
{
    int i32;
    int i = 0;
    if (!entry || !t || !vector)
        return;

    struct config_setting_t *tt = libconfig->setting_get_member(t, fieldName);

    if (!tt || !config_setting_is_group(tt))
        return;

    struct config_setting_t *item;

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

        VECTOR_ENSURE(*vector, 1, 1);
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

static bool craft_read_db_sub(struct config_setting_t *craftt,
                              int id __attribute__ ((unused)),
                              const char *source)
{
    int class_;
    int i32;
    const char *str = NULL;
    struct config_setting_t *t;

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
    entry->id = class_;
    idb_put(craftconf_db, class_, entry);

    safestrncpy(entry->name, str, sizeof(entry->name));

    script->set_constant2(entry->name, entry->id, false, false);

    //ShowInfo("Craft: id=%d, name=%s\n", class_, entry->name);

    readField("Priority", priority, 0);
    readField("Price", price, 0);
    readField("Lv", level, 0);
    readField("Flag", flag, -1);
    readField("ReturnCode", return_code, 0);

    if ((t = libconfig->setting_get_member(craftt, "SourceItems")) && config_setting_is_list(t))
    {
        int i, len = libconfig->setting_length(t);

        for (i = 0; i < len; i++)
        {
            craft_read_source_inventory(entry, libconfig->setting_get_elem(t, i));
        }
    }

    if ((t = libconfig->setting_get_member(craftt, "CreateItems")) && config_setting_is_list(t))
    {
        int i, len = libconfig->setting_length(t);

        for (i = 0; i < len; i++)
        {
            craft_read_create_items(entry, libconfig->setting_get_elem(t, i));
        }
    }

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
    struct config_t craft_db_conf;
    char filepath[300];
    struct config_setting_t *cdb;
    struct config_setting_t *t;
    int i = 0;

    nullpo_retv(filename);
    sprintf(filepath, "%s/%s", map->db_path, filename);

    if (!libconfig->load_file(&craft_db_conf, filepath) ||
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

void do_init_craftconf(void)
{
    craftconf_db = idb_alloc(DB_OPT_RELEASE_BOTH);
    load_craft_db("craft_db.conf");
}

static void delete_craft_entry(struct craft_db_entry *entry)
{
    if (!entry)
        return;
    VECTOR_CLEAR(entry->inventories);
    VECTOR_CLEAR(entry->delete_items);
    VECTOR_CLEAR(entry->required_items);
    VECTOR_CLEAR(entry->required_equips);
    VECTOR_CLEAR(entry->required_skills);
    VECTOR_CLEAR(entry->required_quests);
    const int len = VECTOR_LENGTH(entry->create_items);
    int f;
    for (f = 0; f < len; f ++)
    {
        struct craft_items_collection *collection = &VECTOR_INDEX(
            entry->create_items, f);
        VECTOR_CLEAR(*collection);
    }
    VECTOR_CLEAR(entry->create_items);
}

static int delete_craftconf_sub(union DBKey key __attribute__ ((unused)),
                                struct DBData *data,
                                va_list args __attribute__ ((unused)))
{
    struct craft_db_entry *craft = DB->data2ptr(data);
    if (!craft)
        return 0;

    delete_craft_entry(craft);
    return 0;
}

void do_final_craftconf(void)
{
    craftconf_db->destroy(craftconf_db, delete_craftconf_sub);
}
