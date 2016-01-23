// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_CRAFTCONF
#define EVOL_MAP_CRAFTCONF

#include "common/db.h"

#include "emap/const/craft.h"

#include "emap/struct/itempair.h"

extern struct DBMap *craftconf_db;

VECTOR_STRUCT_DECL(craft_items_collection, struct item_pair);

struct craft_db_inventory
{
    struct item_pair items[craft_inventory_size];
};

struct craft_db_entry
{
    int id;
    char name[32];
    VECTOR_DECL(struct craft_db_inventory) inventories;
    struct craft_items_collection create_items;
    struct craft_items_collection delete_items;
    struct craft_items_collection required_items;
    struct craft_items_collection required_equips;
    struct craft_items_collection required_skills;
    struct craft_items_collection required_quests;
    int priority;
    int price;
    int level;
    int flag;
};

enum craft_field_type
{
    CRAFT_ITEM,
    CRAFT_QUEST,
    CRAFT_SKILL,
    CRAFT_BOOL
};

void do_init_craftconf(void);
void do_final_craftconf(void);

#endif  // EVOL_MAP_CRAFTCONF
