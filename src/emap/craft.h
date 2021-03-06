// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_CRAFT
#define EVOL_MAP_CRAFT

#include "common/db.h"

#include "emap/const/craft.h"

#include "emap/struct/itempair.h"

extern struct DBMap *craftvar_db;

VECTOR_STRUCT_DECL(craft_ids, int);

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
    int entry_id;
};

void do_init_craft(void);
void do_final_craft(void);
bool craft_checkstr(TBL_PC *sd, const char *craftstr);
int str_to_craftvar(TBL_PC *sd, const char *craftstr);
struct craft_vardata *craft_str_to_craft(TBL_PC *sd, const char *craftstr);
void craft_dump(TBL_PC *sd, const int id);
void craft_delete(const int id);
struct craft_slot *craft_get_slot(const int id, const int slot);
bool craft_validate(TBL_PC *sd, const int id);
int craft_find_entry(TBL_PC *sd, const int craftvar, const int flag);
bool craft_use_entry(TBL_PC *sd, const int id);
bool craft_use(TBL_PC *sd, const int id);
int craft_get_entry_code(TBL_PC *sd, const int id);

#endif  // EVOL_MAP_CRAFT
