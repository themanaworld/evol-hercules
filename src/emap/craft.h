// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_CRAFT
#define EVOL_MAP_CRAFT

#include "common/db.h"

extern struct DBMap *craftvar_db;

#define craft_inventory_size 9

struct item_pair
{
    int index;
    int amount;
};

struct skill_pair
{
    int id;
    int level;
};

struct craft_slot
{
    VECTOR_DECL(struct item_pair) items;
};

struct craft_vardata
{
    struct craft_slot slots[craft_inventory_size];
};

struct craft_db_inventory
{
    struct item_pair items[craft_inventory_size];
};

VECTOR_STRUCT_DECL(craft_items_collection, struct item_pair);

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

void do_init_craft(void);
void do_final_craft(void);
bool craft_checkstr(TBL_PC *sd, const char *craftstr);
int str_to_craftvar(TBL_PC *sd, const char *craftstr);
struct craft_vardata *craft_str_to_craft(const char *craftstr);
void craft_dump(TBL_PC *sd, const int id);
void craft_delete(const int id);
struct craft_slot *craft_get_slot(const int id, const int slot);
bool craft_validate(TBL_PC *sd, const int id);

#endif  // EVOL_MAP_CRAFT
