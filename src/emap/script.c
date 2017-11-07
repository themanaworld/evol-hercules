// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/script.h"

#include "plugins/HPMHooking.h"

#include "emap/script.h"
#include "emap/map.h"
#include "emap/data/itemd.h"
#include "emap/data/npcd.h"
#include "emap/struct/itemdext.h"
#include "emap/struct/npcdext.h"
#include "emap/skill_const.h"

#define getExt2() \
    TBL_NPC *nd = NULL; \
    int num = (int)reference_uid(script->add_str(".id"), 0); \
    int id = (int)i64db_iget(n->vars, num); \
    if (!id) \
        id = st->oid; \
    nd = map->id2nd(id); \
    if (!nd) \
        return; \
    struct NpcdExt *ext = npcd_get(nd); \
    if (!ext) \
        return;

#define getExt2Ret(r) \
    TBL_NPC *nd = NULL; \
    int num = (int)reference_uid(script->add_str(".id"), 0); \
    int id = (int)i64db_iget(n->vars, num); \
    if (!id) \
        id = st->oid; \
    nd = map->id2nd(id); \
    if (!nd) \
        return r; \
    struct NpcdExt *ext = npcd_get(nd); \
    if (!ext) \
        return r;

#define getExt1() \
    TBL_NPC *nd = NULL; \
    int num = (int)reference_uid(script->add_str(".id"), 0); \
    int id = (int)i64db_iget(n->vars, num); \
    if (!id) \
        id = st->oid; \
    nd = map->id2nd(id); \
    if (!nd) \
        return; \

#define getExt1Return(r) \
    TBL_NPC *nd = NULL; \
    int num = (int)reference_uid(script->add_str(".id"), 0); \
    int id = (int)i64db_iget(n->vars, num); \
    if (!id) \
        id = st->oid; \
    nd = map->id2nd(id); \
    if (!nd) \
        return r;

int escript_reload_pre(void)
{
    map_clear_data();
    return 0;
}

void escript_load_parameters_pre(void)
{
    script->constdb_comment("Evol parameters");
    script->set_constant("ClientVersion", 10000, true, false);
    script->constdb_comment(NULL);
}

void escript_hardcoded_constants_pre(void)
{
    script->constdb_comment("Evol constants");
    script->set_constant("MAX_SLOTS", MAX_SLOTS, false, false);
    script->constdb_comment(NULL);
    eskill_addskill_conststants();
}

// stripped copy from script_load_translations without actual translation loading.
void escript_load_translations_pre(void)
{
    if (map->minimal)
    {
        hookStop();
        return;
    }

    script->translation_db = strdb_alloc(DB_OPT_DUP_KEY, NAME_LENGTH * 2 + 1);

    if (script->languages)
    {
        int i;
        for (i = 0; i < script->max_lang_id; i++)
            aFree(script->languages[i]);
        aFree(script->languages);
    }
    script->languages = NULL;
    script->max_lang_id = 0;

    script->add_language("English");

    if (script->string_list)
        aFree(script->string_list);

    script->string_list = NULL;
    script->string_list_pos = 0;
    script->string_list_size = 0;

    map->default_lang_id = 0;
    hookStop();
}

void eset_reg_npcscope_num_pre(struct script_state **stPtr,
                               struct reg_db **nPtr,
                               int64 *num __attribute__ ((unused)),
                               const char **namePtr,
                               int *val)
{
    struct script_state *st = *stPtr;
    struct reg_db *n = *nPtr;
    const char *name = *namePtr;

    if (!strcmp(name, ".lang"))
    {
        getExt2();
        ext->language = *val;
        hookStop();
    }
    else if (!strcmp(name, ".sex"))
    {
        getExt1();
        clif->clearunit_area(&nd->bl, CLR_OUTSIGHT);
        nd->vd.sex = *val;
        clif->spawn(&nd->bl);
        hookStop();
    }
    else if (!strcmp(name, ".distance"))
    {
        getExt1();
        nd->area_size = *val;
        hookStop();
    }
    else if (!strcmp(name, ".dir"))
    {
        getExt1();
        int newdir = *val;

        if (newdir < 0)
            newdir = 0;
        else if (newdir > 7)
            newdir = 7;

        nd->dir = newdir;
        npc->enable(nd->exname, 1);
        hookStop();
    }
    else if (!strcmp(name, ".x"))
    {
        ShowWarning("you cant assign '.x'\n");
        script->reportsrc(st);
        hookStop();
    }
    else if (!strcmp(name, ".y"))
    {
        ShowWarning("you cant assign '.y'.\n");
        script->reportsrc(st);
        hookStop();
    }
    else if (!strcmp(name, ".class"))
    {
        getExt1();
        int class_ = *val;
        if (nd->class_ != class_)
            npc->setclass(nd, class_);
        hookStop();
    }
    else if (!strcmp(name, ".speed"))
    {
        getExt1();
        unit->bl2ud2(&nd->bl); // ensure nd->ud is safe to edit
        nd->speed = *val;
        nd->ud->state.speed_changed = 1;
        hookStop();
    }
    else if (!strcmp(name, ".chat"))
    {
        ShowWarning("you cant assign '.chat'.\n");
        script->reportsrc(st);
        hookStop();
    }
    else if (!strcmp(name, ".sit"))
    {
        getExt1();
        nd->vd.dead_sit = (*val) ? 2 : 0;
        clif->sitting(&nd->bl);
        hookStop();
    }
    else if (!strcmp(name, ".stand"))
    {
        getExt1();
        nd->vd.dead_sit = (*val) ? 0 : 2;
        clif->sitting(&nd->bl);
        hookStop();
    }
    else if (!strcmp(name, ".walkmask"))
    {
        getExt2();
        ext->walkMask = *val;
        hookStop();
    }
    else if (!strcmp(name, ".alwaysVisible"))
    {
        getExt1();
        if (*val)
            map_alwaysVisible_add(&nd->bl);
        else
            map_alwaysVisible_delete(&nd->bl);
        hookStop();
    }
    else if (!strcmp(name, ".srcId"))
    {
        ShowWarning("you cant assign '.srcId'\n");
        script->reportsrc(st);
        hookStop();
    }
}

int eget_val_npcscope_num_pre(struct script_state **stPtr,
                              struct reg_db **nPtr,
                              struct script_data **dataPtr)
{
    struct script_state *st = *stPtr;
    struct reg_db *n = *nPtr;
    struct script_data *data = *dataPtr;
    const char *name = reference_getname(data);
    if (!strcmp(name, ".lang"))
    {
        getExt2Ret(0);
        hookStop();
        return ext->language;
    }
    else if (!strcmp(name, ".sex"))
    {
        getExt1Return(0);
        hookStop();
        return nd->vd.sex;
    }
    else if (!strcmp(name, ".distance"))
    {
        getExt1Return(0);
        hookStop();
        return nd->area_size;
    }
    else if (!strcmp(name, ".dir"))
    {
        getExt1Return(0);
        hookStop();
        return nd->dir;
    }
    else if (!strcmp(name, ".x"))
    {
        getExt1Return(0);
        hookStop();
        return nd->bl.x;
    }
    else if (!strcmp(name, ".y"))
    {
        getExt1Return(0);
        hookStop();
        return nd->bl.y;
    }
    else if (!strcmp(name, ".class"))
    {
        getExt1Return(0);
        hookStop();
        return nd->class_;
    }
    else if (!strcmp(name, ".speed"))
    {
        getExt1Return(0);
        hookStop();
        return nd->speed;
    }
    else if (!strcmp(name, ".chat"))
    {
        getExt1Return(0);
        hookStop();
        return nd->chat_id;
    }
    else if (!strcmp(name, ".sit"))
    {
        getExt1Return(0);
        hookStop();
        return nd->vd.dead_sit == 2 ? 1 : 0;
    }
    else if (!strcmp(name, ".stand"))
    {
        getExt1Return(0);
        hookStop();
        return nd->vd.dead_sit == 0 ? 1 : 0;
    }
    else if (!strcmp(name, ".walkmask"))
    {
        getExt2Ret(0);
        hookStop();
        return ext->walkMask;
    }
    else if (!strcmp(name, ".alwaysVisible"))
    {
        getExt1Return(0);
        bool res = map_alwaysVisible_find(&nd->bl);
        hookStop();
        return res;
    }
    else if (!strcmp(name, ".srcId"))
    {
        getExt1Return(0);
        hookStop();
        return nd->src_id;
    }
    return 0;
}

void eset_reg_npcscope_str_pre(struct script_state **stPtr,
                               struct reg_db **nPtr,
                               int64 *num __attribute__ ((unused)),
                               const char **namePtr,
                               const char **strPtr)
{
    struct script_state *st = *stPtr;
    struct reg_db *n = *nPtr;
    const char *name = *namePtr;
    const char *str = *strPtr;

    if (!strcmp(name, ".map$"))
    {
        ShowWarning("you cant assign '.map$'.\n");
        script->reportsrc(st);
        hookStop();
    }
    else if (!strcmp(name, ".name$"))
    {
        getExt1();
        npc->setdisplayname(nd, str);
//      not working because cant sent brodcast with translated npc name. need add for_each function for this.
//        clif->clearunit_area(&nd->bl, CLR_OUTSIGHT);
//        safestrncpy(nd->name, str, sizeof(nd->name));
//        clif->spawn(&nd->bl);
        hookStop();
    }
    else if (!strcmp(name, ".extname$"))
    {
        ShowWarning("you cant assign '.extname$'.\n");
        script->reportsrc(st);
        hookStop();
    }
}

char *eget_val_npcscope_str_pre(struct script_state **stPtr,
                                struct reg_db **nPtr,
                                struct script_data **dataPtr)
{
    struct script_state *st = *stPtr;
    struct reg_db *n = *nPtr;
    struct script_data *data = *dataPtr;
    const char *name = reference_getname(data);
    if (!strcmp(name, ".map$"))
    {
        getExt1Return(0);
        hookStop();
        return map->list[nd->bl.m].name;
    }
    else if (!strcmp(name, ".name$"))
    {
        getExt1Return(0);
        hookStop();
        return nd->name;
    }
    else if (!strcmp(name, ".extname$"))
    {
        getExt1Return(0);
        hookStop();
        return nd->exname;
    }
    return NULL;
}

void escript_run_use_script_pre(struct map_session_data **sdPtr,
                                struct item_data **itemDataPtr,
                                int *oidPtr)
{
    nullpo_retv(*itemDataPtr);
    struct map_session_data *sd = *sdPtr;
    struct item_data *itemData = *itemDataPtr;
    const int oid = *oidPtr;
    if (oid == 0)
    {
        pc->setreg(sd, script->add_str("@useType"), 0);
        script->current_item_id = itemData->nameid;
        script->run(itemData->script, 0, sd->bl.id, oid);
        script->current_item_id = 0;
    }
    else
    {
        struct ItemdExt *data = itemd_get(itemData);
        if (!data)
        {
            data->tmpUseType = 0;
            hookStop();
            return;
        }

        pc->setreg(sd, script->add_str("@useType"), data->tmpUseType);
        script->current_item_id = itemData->nameid;
        script->run(itemData->script, 0, sd->bl.id, oid);
        script->current_item_id = 0;
        pc->setreg(sd, script->add_str("@useType"), 0);
        data->tmpUseType = 0;
    }
    hookStop();
}

void script_run_item_amount_script(TBL_PC *sd,
                                   struct script_code *itemScript,
                                   int itemId,
                                   int amount)
{
    if (!itemScript)
        return;

    script->current_item_id = itemId;
    pc->setreg(sd, script->add_str("@itemId"), itemId);
    pc->setreg(sd, script->add_str("@itemAmount"), amount);
    script->run(itemScript, 0, sd->bl.id, npc->fake_nd->bl.id);
    pc->setreg(sd, script->add_str("@itemId"), 0);
    pc->setreg(sd, script->add_str("@itemAmount"), 0);
    script->current_item_id = 0;
}

void script_run_card_script(TBL_PC *sd,
                            struct script_code *itemScript,
                            int itemId,
                            int cardId)
{
    if (!itemScript)
        return;

    script->current_item_id = itemId;
    pc->setreg(sd, script->add_str("@itemId"), itemId);
    pc->setreg(sd, script->add_str("@cardId"), cardId);
    script->run(itemScript, 0, sd->bl.id, npc->fake_nd->bl.id);
    pc->setreg(sd, script->add_str("@itemId"), 0);
    pc->setreg(sd, script->add_str("@cardId"), 0);
    script->current_item_id = 0;
}
