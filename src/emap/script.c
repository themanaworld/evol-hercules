// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/utils.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/chat.h"
#include "map/chrif.h"
#include "map/clif.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/script.h"
#include "map/quest.h"
#include "map/unit.h"

#include "emap/script.h"
#include "emap/clif.h"
#include "emap/craft.h"
#include "emap/lang.h"
#include "emap/map.h"
#include "emap/scriptdefines.h"
#include "emap/send.h"
#include "emap/data/bgd.h"
#include "emap/data/mapd.h"
#include "emap/data/npcd.h"
#include "emap/data/session.h"
#include "emap/struct/bgdext.h"
#include "emap/struct/mapdext.h"
#include "emap/struct/npcdext.h"
#include "emap/struct/sessionext.h"
#include "emap/utils/formatutils.h"

#define getExt2() \
    TBL_NPC *nd = NULL; \
    int num = reference_uid(script->add_str(".id"), 0); \
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
    int num = reference_uid(script->add_str(".id"), 0); \
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
    int num = reference_uid(script->add_str(".id"), 0); \
    int id = (int)i64db_iget(n->vars, num); \
    if (!id) \
        id = st->oid; \
    nd = map->id2nd(id); \
    if (!nd) \
        return; \

#define getExt1Return(r) \
    TBL_NPC *nd = NULL; \
    int num = reference_uid(script->add_str(".id"), 0); \
    int id = (int)i64db_iget(n->vars, num); \
    if (!id) \
        id = st->oid; \
    nd = map->id2nd(id); \
    if (!nd) \
        return r;

extern int mountScriptId;

int escript_reload(void)
{
    map_clear_data();
    return 0;
}

void escript_load_parameters(void)
{
    script->constdb_comment("Evol parameters");
    script->set_constant("ClientVersion", 10000, true, false);
    script->constdb_comment(NULL);
}

// stripped copy from script_load_translations without actual translation loading.
void escript_load_translations(void)
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

void eset_reg_npcscope_num(struct script_state* st, struct reg_db *n, int64 *num, const char* name, int *val)
{
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
        nd->vd->sex = *val;
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
        npc->enable(nd->name, 1);
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
        nd->vd->dead_sit = (*val) ? 2 : 0;
        clif->sitting(&nd->bl);
        hookStop();
    }
    else if (!strcmp(name, ".stand"))
    {
        getExt1();
        nd->vd->dead_sit = (*val) ? 0 : 2;
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
}

int eget_val_npcscope_num(struct script_state* st, struct reg_db *n, struct script_data* data)
{
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
        return nd->vd->sex;
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
        return nd->vd->dead_sit == 2 ? 1 : 0;
    }
    else if (!strcmp(name, ".stand"))
    {
        getExt1Return(0);
        hookStop();
        return nd->vd->dead_sit == 0 ? 1 : 0;
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
    return 0;
}

void eset_reg_npcscope_str(struct script_state* st, struct reg_db *n, int64 *num, const char* name, const char *str)
{
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

char *eget_val_npcscope_str(struct script_state* st, struct reg_db *n, struct script_data* data)
{
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

void script_run_item_amount_script(TBL_PC *sd, struct script_code *itemScript, int itemId, int amount)
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

void script_run_card_script(TBL_PC *sd, struct script_code *itemScript, int itemId, int cardId)
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

uint32 MakeDWord(uint16 word0, uint16 word1)
{
    return ((uint32)(word0)) | ((uint32)(word1 << 0x10));
}

BUILDIN(l)
{
    format_sub(st, 1);
    return true;
}

BUILDIN(lg)
{
    format_sub(st, 2);
    return true;
}

BUILDIN(setCamNpc)
{
    getSD();
    TBL_NPC *nd = NULL;

    int x = 0;
    int y = 0;

    if (script_hasdata(st, 2))
    {
        nd = npc->name2id (script_getstr(st, 2));
    }
    else
    {
        if (!st->oid)
        {
            ShowWarning("npc not attached\n");
            script->reportsrc(st);
            return false;
        }

        nd = map->id2nd(st->oid);
    }
    if (!nd)
    {
        ShowWarning("npc not found\n");
        script->reportsrc(st);
        return false;
    }
    if (sd->bl.m != nd->bl.m)
    {
        ShowWarning("npc and player located in other maps\n");
        script->reportsrc(st);
        return false;
    }

    if (script_hasdata(st, 3) && script_hasdata(st, 4))
    {
        x = script_getnum(st, 3);
        y = script_getnum(st, 4);
    }

    send_npccommand2(sd, st->oid, 2, nd->bl.id, x, y);

    return true;
}

BUILDIN(setCam)
{
    send_npccommand2(script->rid2sd (st), st->oid, 2, 0,
        script_getnum(st, 2), script_getnum(st, 3));

    return true;
}

BUILDIN(moveCam)
{
    send_npccommand2(script->rid2sd (st), st->oid, 4, 0,
        script_getnum(st, 2), script_getnum(st, 3));

    return true;
}

BUILDIN(restoreCam)
{
    getSD();
    send_npccommand(sd, st->oid, 3);
    return true;
}

BUILDIN(npcTalk3)
{
    const char *msg;

    getSD();

    TBL_NPC *nd = map->id2nd(st->oid);
    const char *str = script_getstr(st, 2);

    if (!nd)
    {
        ShowWarning("npc not found\n");
        script->reportsrc(st);
        return false;
    }

    if (!str)
    {
        ShowWarning("error in string\n");
        script->reportsrc(st);
        return false;
    }

    if (sd)
        msg = lang_pctrans (nd->name, sd);
    else
        msg = nd->name;

    if (!msg)
    {
        ShowWarning("error in string\n");
        script->reportsrc(st);
        return false;
    }
    if (strlen(str) + strlen(msg) > 450)
    {
        ShowWarning("text message too big\n");
        script->reportsrc(st);
        return false;
    }

    if (nd)
    {
        char message[500];
        char name[500];
        strcpy (name, msg);
        strtok(name, "#");
        strcpy (message, name);
        strcat (message, " : ");
        strcat (message, str);
        send_local_message (sd->fd, &(nd->bl), message);
    }

    return true;
}

BUILDIN(closeDialog)
{
    getSD();
    send_npccommand(sd, st->oid, 5);
    return true;
}

BUILDIN(shop)
{
    getSD();
    TBL_NPC *nd = npc->name2id (script_getstr(st, 2));
    if (!nd)
    {
        ShowWarning("shop npc not found\n");
        script->reportsrc(st);
        return false;
    }

    st->state = sd->state.dialog == 1 ? CLOSE : END;
    clif->scriptclose(sd, st->oid);

    clif->npcbuysell (sd, nd->bl.id);
    return true;
}

#define paramToItem(param) \
    if (script_isstringtype(st, param)) \
    { \
        i_data = itemdb->search_name (script_getstr(st, param)); \
    } \
    else \
    { \
        item_id = script_getnum (st, param); \
        if (item_id) \
            i_data = itemdb->search (item_id); \
    }

BUILDIN(getItemLink)
{
    struct item_data *i_data = NULL;
    char *item_name;
    int  item_id = 0;

    paramToItem(2);

    item_name = (char *) aCalloc (1000, sizeof (char));
    TBL_PC *sd = script->rid2sd(st);

    if (sd)
    {
        int version = 0;
        struct SessionExt *data = session_get_bysd(sd);
        if (data)
            version = data->clientVersion;

        if (i_data && version >= 7)
        {
            if (!script_hasdata(st, 3))
            {
                sprintf(item_name, "[@@%u|@@]", (unsigned)i_data->nameid);
            }
            else
            {
                sprintf(item_name, "[@@%u", (unsigned)i_data->nameid);
                int f;
                for (f = 3; f < 7 && script_hasdata(st, f); f ++)
                {
                    paramToItem(f);
                    if (i_data)
                    {
                        char buf[100];
                        sprintf(buf, ",%u", (unsigned)i_data->nameid);
                        strcat(item_name, buf);
                    }
                }
                strcat(item_name, "|@@]");
            }
        }
        else if (i_data)
        {
            sprintf(item_name, "[@@%u|%s@@]", (unsigned)i_data->nameid, lang_pctrans (i_data->jname, sd));
        }
        else if (item_id > 0)
        {
            sprintf(item_name, "[@@%u|Unknown Item@@]", (unsigned)item_id);
        }
        else
        {
            sprintf(item_name, "[Unknown Item]");
        }
    }
    else
    {
        if (i_data)
            sprintf(item_name, "[%s]", lang_pctrans (i_data->jname, sd));
        else
            sprintf(item_name, "[Unknown Item]");
    }

    script_pushstr(st, item_name);

    return true;
}

#undef paramToItem

BUILDIN(requestLang)
{
    getSDReturn(-1);

    if (!sd->state.menu_or_input)
    {
        // first invocation, display npc input box
        sd->state.menu_or_input = 1;
        st->state = RERUNLINE;

        // send lang request
        send_npccommand(sd, st->oid, 0);
        clif->scriptinputstr(sd, st->oid);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;

        int lng = -1;
        if (*sd->npc_str)
            lng = lang_getId(sd->npc_str);
        script_pushint(st, lng);
        st->state = RUN;
    }
    return true;
}

BUILDIN(requestItem)
{
    getSDReturn(0);

    if (!sd->state.menu_or_input)
    {
        // first invocation, display npc input box
        sd->state.menu_or_input = 1;
        st->state = RERUNLINE;

        // send item request
        send_npccommand(sd, st->oid, 10);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;

        int item = 0;

        if (!sd->npc_str)
        {
            ShowWarning("npc string not found\n");
            script_pushint(st, 0);
            script->reportsrc(st);
            return false;
        }

        if (sscanf (sd->npc_str, "%5d", &item) < 1)
        {
            ShowWarning("input data is not item id\n");
            script_pushint(st, 0);
            script->reportsrc(st);
            return false;
        }

        script_pushint(st, item);
        st->state = RUN;
    }
    return true;
}

BUILDIN(requestItems)
{
    getSDReturnS("0,0");
    int count = 1;

    if (script_hasdata(st, 2))
    {
        count = script_getnum(st, 2);
        if (count < 0)
            count = 1;
    }

    if (!sd->state.menu_or_input)
    {
        // first invocation, display npc input box
        sd->state.menu_or_input = 1;
        st->state = RERUNLINE;

        // send item request with limit count
        send_npccommand2(sd, st->oid, 10, count, 0, 0);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;

        if (!sd->npc_str)
        {
            ShowWarning("npc string not found\n");
            script_pushstr(st, aStrdup("0,0"));
            script->reportsrc(st);
            return false;
        }

        script_pushstr(st, aStrdup(sd->npc_str));
        st->state = RUN;
    }
    return true;
}

BUILDIN(requestItemIndex)
{
    getSessionDataReturn(client, -1);

    if (!sd->state.menu_or_input)
    {
        // first invocation, display npc input box
        sd->state.menu_or_input = 1;
        st->state = RERUNLINE;

        // send item request
        if (client || client->clientVersion >= 11)
            send_npccommand(sd, st->oid, 11);
        else
            clif->scriptinputstr(sd, st->oid);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;

        int item = -1;

        if (!sd->npc_str)
        {
            script_pushint(st, -1);
            ShowWarning("npc string not found\n");
            script->reportsrc(st);
            return false;
        }

        if (sscanf (sd->npc_str, "%5d", &item) < 1)
        {
            script_pushint(st, -1);
            ShowWarning("input data is not item id\n");
            script->reportsrc(st);
            return false;
        }

        script_pushint(st, item);
        st->state = RUN;
    }
    return true;
}

BUILDIN(requestItemsIndex)
{
    getSessionDataReturnS(client, "-1");
    int count = 1;

    if (script_hasdata(st, 2))
    {
        count = script_getnum(st, 2);
        if (count < 0)
            count = 1;
    }

    if (!sd->state.menu_or_input)
    {
        // first invocation, display npc input box
        sd->state.menu_or_input = 1;
        st->state = RERUNLINE;

        // send item request with limit count
        if (client || client->clientVersion >= 11)
            send_npccommand2(sd, st->oid, 11, count, 0, 0);
        else
            clif->scriptinputstr(sd, st->oid);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;

        if (!sd->npc_str)
        {
            script_pushstr(st, aStrdup("-1"));
            ShowWarning("npc string not found\n");
            script->reportsrc(st);
            return false;
        }

        script_pushstr(st, aStrdup(sd->npc_str));
        st->state = RUN;
    }
    return true;
}

BUILDIN(requestCraft)
{
    getSessionData(client);

    int count = 1;

    if (script_hasdata(st, 2))
    {
        count = script_getnum(st, 2);
        if (count < 0)
            count = 1;
    }

    if (!sd->state.menu_or_input)
    {
        // first invocation, display npc input box
        sd->state.menu_or_input = 1;
        st->state = RERUNLINE;

        // send item request with limit count
        if (client || client->clientVersion >= 16)
            send_npccommand2(sd, st->oid, 12, count, 0, 0);
        else
            clif->scriptinputstr(sd, st->oid);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;

        if (!sd->npc_str)
        {
            ShowWarning("npc string not found\n");
            script->reportsrc(st);
            script_pushstr(st, aStrdup(""));
            return false;
        }

        script_pushstr(st, aStrdup(sd->npc_str));
        st->state = RUN;
    }
    return true;
}

BUILDIN(setq)
{
    int i;
    getSD();

    int quest_id = script_getnum(st, 2);
    int quest_value = script_getnum(st, 3);

    if (quest->check(sd, quest_id, HAVEQUEST) < 0)
        quest->add(sd, quest_id);
    ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
    if (i == sd->num_quests)
    {
        ShowError("Quest with id=%d not found\n", quest_id);
        script->reportsrc(st);
        return false;
    }

    sd->quest_log[i].count[0] = quest_value;
    sd->save_quest = true;
    if (map->save_settings & 64)
        chrif->save(sd,0);

    eclif_quest_add(sd, &sd->quest_log[i]);
    return true;
}

BUILDIN(getq)
{
    int i;
    getSDReturn(0);

    int quest_id = script_getnum(st, 2);
    if (quest->check(sd, quest_id, HAVEQUEST) < 0)
    {
        script_pushint(st, 0);
        return true;
    }
    ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
    if (i == sd->num_quests)
    {
        script_pushint(st, 0);
        return true;
    }
    script_pushint(st, sd->quest_log[i].count[0]);
    return true;
}

BUILDIN(setNpcDir)
{
    int newdir;
    TBL_NPC *nd = 0;

    if (script_hasdata(st, 3))
    {
        nd = npc->name2id (script_getstr(st, 2));
        newdir = script_getnum(st, 3);
    }
    else if (script_hasdata(st, 2))
    {
        if (!st->oid)
        {
            ShowWarning("npc not found\n");
            script->reportsrc(st);
            return false;
        }

        nd = map->id2nd(st->oid);
        newdir = script_getnum(st, 2);
    }
    if (!nd)
    {
        ShowWarning("npc not found\n");
        script->reportsrc(st);
        return false;
    }

    if (newdir < 0)
        newdir = 0;
    else if (newdir > 7)
        newdir = 7;

    nd->dir = newdir;
    npc->enable (nd->name, 1);

    return true;
}

BUILDIN(rif)
{
    const char *str = 0;
    if (script_getnum(st, 2))
    {
        str = script_getstr(st, 3);
        if (str)
            script_pushstr(st, aStrdup(str));
        else
            script_pushconststr(st, (char *)"");
    }
    else if (script_hasdata(st, 4))
    {
        str = script_getstr(st, 4);
        if (str)
            script_pushstr(st, aStrdup(str));
        else
            script_pushconststr(st, (char *)"");
    }
    else
    {
        script_pushconststr(st, (char *)"");
    }

    return true;
}

BUILDIN(miscEffect)
{
    int type = script_getnum(st, 2);
    struct block_list *bl = NULL;

    if (script_hasdata(st, 3))
    {
        if (script_isstring(st, 3))
        {
            TBL_PC *sd = map->nick2sd(script_getstr(st, 3));
            if (sd)
                bl = &sd->bl;
        }
        else if (script_isint(st, 3))
        {
            bl = map->id2bl(script_getnum(st, 3));
        }
    }

    if (!bl)
    {
        TBL_PC *sd = script->rid2sd (st);
        if (sd)
            bl = &sd->bl;
    }
    if (bl)
        clif->specialeffect(bl, type, AREA);
    return true;
}

BUILDIN(setMapMask)
{
    const char *const mapName = script_getstr(st, 2);
    if (!mapName)
    {
        ShowWarning("invalid map name\n");
        script->reportsrc(st);
        return false;
    }
    const int m = map->mapname2mapid(mapName);
    if (m < 0)
    {
        ShowWarning("map not found\n");
        script->reportsrc(st);
        return false;
    }
    getMapData(m);

    const int val = script_getnum(st, 3);
    const unsigned int old = mapData->mask;
    mapData->mask = val;
    if (old != mapData->mask)
        send_mapmask_brodcast(m, mapData->mask);
    return true;
}

BUILDIN(getMapMask)
{
    const char *const mapName = script_getstr(st, 2);
    if (!mapName)
    {
        script_pushint(st, 0);
        ShowWarning("invalid map name\n");
        script->reportsrc(st);
        return false;
    }
    const int m = map->mapname2mapid(mapName);
    if (m < 0)
    {
        script_pushint(st, 0);
        ShowWarning("map not found\n");
        script->reportsrc(st);
        return false;
    }
    getMapDataReturn(m, 0);
    script_pushint(st, mapData->mask);
    return true;
}

BUILDIN(addMapMask)
{
    const char *const mapName = script_getstr(st, 2);
    if (!mapName)
    {
        ShowWarning("invalid map name\n");
        script->reportsrc(st);
        return false;
    }
    const int m = map->mapname2mapid(mapName);
    if (m < 0)
    {
        ShowWarning("map not found\n");
        script->reportsrc(st);
        return false;
    }
    getMapData(m);
    const int val = script_getnum(st, 3);
    const unsigned int old = mapData->mask;
    mapData->mask |= val;
    if (old != mapData->mask)
        send_mapmask_brodcast(m, mapData->mask);

    return true;
}

BUILDIN(removeMapMask)
{
    const char *const mapName = script_getstr(st, 2);
    if (!mapName)
    {
        ShowWarning("invalid map name\n");
        script->reportsrc(st);
        return true;
    }
    const int m = map->mapname2mapid(mapName);
    if (m < 0)
    {
        ShowWarning("map not found\n");
        script->reportsrc(st);
        return false;
    }
    getMapData(m);
    const int val = script_getnum(st, 3);
    const unsigned int old = mapData->mask;
    mapData->mask |= val;
    mapData->mask ^= val;
    if (old != mapData->mask)
        send_mapmask_brodcast(m, mapData->mask);
    return true;
}

BUILDIN(setNpcSex)
{
    TBL_NPC *nd = NULL;
    int sex = 0;
    if (script_hasdata(st, 3))
    {
        nd = npc->name2id (script_getstr(st, 2));
        sex = script_getnum(st, 3);
    }

    if (!nd || !nd->vd)
    {
        ShowWarning("npc not found\n");
        script->reportsrc(st);
        return false;
    }

    clif->clearunit_area(&nd->bl, CLR_OUTSIGHT);
    nd->vd->sex = sex;
    clif->spawn(&nd->bl);
    return true;
}

BUILDIN(showAvatar)
{
    int id = 0;
    if (script_hasdata(st, 2))
        id = script_getnum(st, 2);

    send_npccommand2(script->rid2sd (st), st->oid, 6, id, 0, 0);
    return true;
}

BUILDIN(setAvatarDir)
{
    int newdir = script_getnum(st, 2);

    if (newdir < 0)
        newdir = 0;
    else if (newdir > 7)
        newdir = 7;

    send_npccommand2(script->rid2sd (st), st->oid, 7, newdir, 0, 0);
    return true;
}

BUILDIN(setAvatarAction)
{
    send_npccommand2(script->rid2sd (st), st->oid, 8, script_getnum(st, 2), 0, 0);
    return true;
}

BUILDIN(clear)
{
    send_npccommand(script->rid2sd (st), st->oid, 9);
    return true;
}

BUILDIN(changeMusic)
{
    const char *const mapName = script_getstr(st, 2);
    const char *const music = script_getstr(st, 3);
    if (!music)
    {
        ShowWarning("invalid music file\n");
        script->reportsrc(st);
        return false;
    }
    if (!mapName)
    {
        ShowWarning("invalid map file\n");
        script->reportsrc(st);
        return false;
    }
    const int m = map->mapname2mapid(mapName);
    if (m < 0)
    {
        ShowWarning("map not found\n");
        script->reportsrc(st);
        return false;
    }

    send_changemusic_brodcast(m, music);
    return true;
}

BUILDIN(setNpcDialogTitle)
{
    const char *const name = script_getstr(st, 2);
    if (!name)
    {
        ShowWarning("invalid window title\n");
        script->reportsrc(st);
        return false;
    }
    TBL_PC *sd = script->rid2sd (st);
    if (!sd)
    {
        ShowWarning("player not attached\n");
        script->reportsrc(st);
        return false;
    }

    send_changenpc_title(sd, st->oid, name);
    return true;
}

BUILDIN(getMapName)
{
    TBL_PC *sd = script->rid2sd(st);
    if (!sd)
    {
        script_pushstr(st, aStrdup(""));
        ShowWarning("player not attached\n");
        script->reportsrc(st);
        return false;
    }
    if (sd->bl.m == -1)
    {
        script_pushstr(st, aStrdup(""));
        ShowWarning("invalid map\n");
        script->reportsrc(st);
        return false;
    }
    script_pushstr(st, aStrdup(map->list[sd->bl.m].name));
    return true;
}

BUILDIN(unequipById)
{
    int nameid = 0;
    int i;
    struct item_data *item_data;
    TBL_PC *sd = script->rid2sd(st);

    if (sd == NULL)
    {
        ShowWarning("player not attached\n");
        script->reportsrc(st);
        return false;
    }

    nameid = script_getnum(st, 2);
    if((item_data = itemdb->exists(nameid)) == NULL)
    {
        ShowWarning("item %d not found\n", nameid);
        script->reportsrc(st);
        return false;
    }
    for (i = 0; i < EQI_MAX; i++)
    {
        const int idx = sd->equip_index[i];
        if (idx >= 0)
        {
            if (sd->status.inventory[idx].nameid == nameid)
                pc->unequipitem(sd, idx, 1 | 2);
        }
    }
    return true;
}

BUILDIN(isPcDead)
{
    TBL_PC *sd = script->rid2sd(st);

    if (sd == NULL)
    {
        ShowWarning("player not attached\n");
        script->reportsrc(st);
        return false;
    }

    script_pushint(st, pc_isdead(sd) ? 1 : 0);
    return true;
}

static int areatimer_sub(struct block_list *bl, va_list ap)
{
    int tick;
    char *event;
    TBL_PC *sd;

    tick = va_arg(ap, int);
    event = va_arg(ap, char*);

    sd = (TBL_PC *)bl;
    if (!pc->addeventtimer(sd, tick, event))
    {
        if (sd)
            ShowWarning("buildin_addtimer: Event timer is full, can't add new event timer. (cid:%d timer:%s)\n", sd->status.char_id, event);
    }
    return 0;
}

BUILDIN(areaTimer)
{
    const char *const mapname = script_getstr(st, 2);
    const int x1 = script_getnum(st, 3);
    const int y1 = script_getnum(st, 4);
    const int x2 = script_getnum(st, 5);
    const int y2 = script_getnum(st, 6);
    const int time = script_getnum(st, 7);
    const char *const eventName = script_getstr(st, 8);
    int m;

    if ((m = map->mapname2mapid(mapname)) < 0)
    {
        ShowWarning("map not found\n");
        script->reportsrc(st);
        return false;
    }

    map->foreachinarea(areatimer_sub, m, x1, y1, x2, y2, BL_PC, time, eventName);

    return true;
}

static int buildin_getareadropitem_sub_del(struct block_list *bl, va_list ap)
{
    if (!bl)
        return 0;

    const int item = va_arg(ap, int);
    int *const amount = va_arg(ap, int *);
    TBL_ITEM *drop = (TBL_ITEM *)bl;

    if (drop->item_data.nameid == item)
    {
        (*amount) += drop->item_data.amount;
        map->clearflooritem(&drop->bl);
    }

    return 0;
}

BUILDIN(getAreaDropItem)
{
    const char *const str = script_getstr(st, 2);
    int16 m;
    int x0 = script_getnum(st, 3);
    int y0 = script_getnum(st, 4);
    int x1 = script_getnum(st, 5);
    int y1 = script_getnum(st, 6);
    int item;
    int amount = 0;

    if (script_isstringtype(st, 7))
    {
        const char *name = script_getstr(st, 7);
        struct item_data *item_data = itemdb->search_name(name);
        item = UNKNOWN_ITEM_ID;
        if (item_data)
            item = item_data->nameid;
    }
    else
    {
        item = script_getnum(st, 7);
    }

    if ((m = map->mapname2mapid(str)) < 0)
    {
        script_pushint(st, -1);
        ShowWarning("map not found\n");
        script->reportsrc(st);
        return false;
    }

    if (script_hasdata(st, 8) && script_getnum(st, 8))
    {
        map->foreachinarea(buildin_getareadropitem_sub_del,
            m, x0, y0, x1, y1, BL_ITEM, item, &amount);
    }
    else
    {
        map->foreachinarea(script->buildin_getareadropitem_sub,
            m, x0, y0, x1, y1, BL_ITEM, item, &amount);
    }
    script_pushint(st, amount);
    return true;
}

BUILDIN(clientCommand)
{
    getSD();

    const char *const command = script_getstr(st, 2);
    if (!command)
    {
        ShowWarning("invalid client command\n");
        script->reportsrc(st);
        return false;
    }
    send_client_command(sd, command);
    return true;
}

BUILDIN(isUnitWalking)
{
    int id = 0;
    if (script_hasdata(st, 2))
        id = script_getnum(st, 2);
    else
        id = st->oid;
    struct block_list *bl = map->id2bl(id);
    if (!bl)
    {
        ShowWarning("invalid unit id\n");
        script->reportsrc(st);
        script_pushint(st, 0);
        return false;
    }
    struct unit_data *ud = unit->bl2ud(bl);
    if (!ud)
    {
        ShowWarning("invalid unit data\n");
        script->reportsrc(st);
        script_pushint(st, 0);
        return false;
    }
    script_pushint(st, ud->walktimer != INVALID_TIMER);
    return true;
}

BUILDIN(failedRefIndex)
{
    getSD()
    getInventoryIndex(2)

    if (sd->status.inventory[n].nameid <= 0 || sd->status.inventory[n].amount <= 0)
        return false;

    sd->status.inventory[n].refine = 0;
    if (sd->status.inventory[n].equip)
        pc->unequipitem(sd, n, PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE);
    clif->refine(sd->fd, 1, n, sd->status.inventory[n].refine);
    pc->delitem(sd, n, 1, 0, DELITEM_FAILREFINE, LOG_TYPE_SCRIPT);
    clif->misceffect(&sd->bl, 2);
    return true;
}

BUILDIN(downRefIndex)
{
    getSD()
    getInventoryIndex(2)

    if (sd->status.inventory[n].nameid <= 0 || sd->status.inventory[n].amount <= 0)
        return false;

    const int down = script_getnum(st, 3);

    logs->pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->status.inventory[n], sd->inventory_data[n]);

    if (sd->status.inventory[n].equip)
        pc->unequipitem(sd, n, PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE);
    sd->status.inventory[n].refine -= down;
    sd->status.inventory[n].refine = cap_value(sd->status.inventory[n].refine, 0, MAX_REFINE);

    clif->refine(sd->fd, 2, n, sd->status.inventory[n].refine);
    clif->delitem(sd, n, 1, DELITEM_MATERIALCHANGE);
    logs->pick_pc(sd, LOG_TYPE_SCRIPT, 1, &sd->status.inventory[n], sd->inventory_data[n]);
    clif->additem(sd, n, 1, 0);
    clif->misceffect(&sd->bl, 2);
    return true;
}

BUILDIN(successRefIndex)
{
    getSD()
    getInventoryIndex(2)

    if (sd->status.inventory[n].nameid <= 0 || sd->status.inventory[n].amount <= 0)
        return false;

    const int up = script_getnum(st, 3);

    logs->pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->status.inventory[n],sd->inventory_data[n]);

    if (sd->status.inventory[n].refine >= MAX_REFINE)
        return true;

    sd->status.inventory[n].refine += up;
    sd->status.inventory[n].refine = cap_value( sd->status.inventory[n].refine, 0, MAX_REFINE);
    if (sd->status.inventory[n].equip)
        pc->unequipitem(sd, n, PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE);
    clif->refine(sd->fd, 0, n, sd->status.inventory[n].refine);
    clif->delitem(sd, n, 1, DELITEM_MATERIALCHANGE);
    logs->pick_pc(sd, LOG_TYPE_SCRIPT, 1, &sd->status.inventory[n],sd->inventory_data[n]);
    clif->additem(sd, n, 1, 0);
    clif->misceffect(&sd->bl, 3);

    if (sd->status.inventory[n].refine == 10 &&
        sd->status.inventory[n].card[0] == CARD0_FORGE &&
        sd->status.char_id == (int)MakeDWord(sd->status.inventory[n].card[2], sd->status.inventory[n].card[3]))
    { // Fame point system [DracoRPG]
        switch (sd->inventory_data[n]->wlv)
        {
            case 1:
                pc->addfame(sd,1); // Success to refine to +10 a lv1 weapon you forged = +1 fame point
                break;
            case 2:
                pc->addfame(sd,25); // Success to refine to +10 a lv2 weapon you forged = +25 fame point
                break;
            case 3:
                pc->addfame(sd,1000); // Success to refine to +10 a lv3 weapon you forged = +1000 fame point
                break;
        }
    }

    return true;
}

// return paramater type
// 0 - int
// 1 - string
// 2 - other
BUILDIN(isStr)
{
    if (script_isinttype(st, 2))
        script_pushint(st, 0);
    else if (script_isstringtype(st, 2))
        script_pushint(st, 1);
    else
        script_pushint(st, 2);
    return true;
}

BUILDIN(npcSit)
{
    TBL_NPC *nd = NULL;

    if (script_hasdata(st, 2))
    {
        nd = npc->name2id (script_getstr(st, 2));
    }
    else
    {
        if (!st->oid)
        {
            ShowWarning("npc not attached\n");
            script->reportsrc(st);
            return false;
        }

        nd = map->id2nd(st->oid);
    }
    if (!nd)
    {
        ShowWarning("npc not found\n");
        script->reportsrc(st);
        return false;
    }
    nd->vd->dead_sit = 2;
    clif->sitting(&nd->bl);
    return true;
}

BUILDIN(npcStand)
{
    TBL_NPC *nd = NULL;

    if (script_hasdata(st, 2))
    {
        nd = npc->name2id (script_getstr(st, 2));
    }
    else
    {
        if (!st->oid)
        {
            ShowWarning("npc not attached\n");
            script->reportsrc(st);
            return false;
        }

        nd = map->id2nd(st->oid);
    }
    if (!nd)
    {
        ShowWarning("npc not found\n");
        script->reportsrc(st);
        return false;
    }
    nd->vd->dead_sit = 0;
    clif->standing(&nd->bl);
    return true;
}

BUILDIN(npcWalkTo)
{
    struct npc_data *nd = map->id2nd(st->oid);
    int x = 0, y = 0;

    x = script_getnum(st, 2);
    y = script_getnum(st, 3);

    if (nd)
    {
        unit->bl2ud2(&nd->bl); // ensure nd->ud is safe to edit
        if (!nd->status.hp)
        {
            status_calc_npc(nd, SCO_FIRST);
        }
        else
        {
            status_calc_npc(nd, SCO_NONE);
        }
        nd->vd->dead_sit = 0;
        unit->walktoxy(&nd->bl,x,y,0);
    }
    else
    {
        ShowWarning("npc not found\n");
        script->reportsrc(st);
        return false;
    }

    return true;
}

BUILDIN(setBgTeam)
{
    int bgId = script_getnum(st, 2);
    int teamId = script_getnum(st, 3);

    struct battleground_data *bgd = bg->team_search(bgId);
    struct BgdExt *data = bgd_get(bgd);
    if (!data)
    {
        ShowWarning("bettle ground not found\n");
        script->reportsrc(st);
        return false;
    }

    data->teamId = teamId;
    return true;
}

// chatjoin chatId [,char [,password]]
BUILDIN(chatJoin)
{
    int chatId = script_getnum(st, 2);
    TBL_PC *sd = NULL;
    const char *password = "";
    if (script_hasdata(st, 4))
    {
        if (script_isstringtype(st, 3))
            sd = map->nick2sd(script_getstr(st, 3));
        if (script_isstringtype(st, 4))
            password = script_getstr(st, 4);
    }
    else if (script_hasdata(st, 3))
    {
        if (script_isstringtype(st, 3))
            sd = map->nick2sd(script_getstr(st, 3));
    }
    else
    {
        sd = script->rid2sd(st);
    }
    if (!sd)
    {
        ShowWarning("player not found\n");
        script->reportsrc(st);
        return false;
    }

    chat->join(sd, chatId, password);
    return true;
}

/// Retrieves the value of the specified flag of the specified cell.
///
/// checknpccell("<map name>",<x>,<y>,<type>) -> <bool>
///
/// @see cell_chk* constants in const.txt for the types
BUILDIN(checkNpcCell)
{
    int16 m = map->mapname2mapid(script_getstr(st, 2));
    int16 x = script_getnum(st, 3);
    int16 y = script_getnum(st, 4);
    cell_chk type = (cell_chk)script_getnum(st, 5);

    if (m == -1)
    {
        ShowWarning("checknpccell: Attempted to run on unexsitent map '%s', type %u, x/y %d,%d\n", script_getstr(st, 2), type, x, y);
        return true;
    }

    TBL_NPC *nd = map->id2nd(st->oid);
    struct block_list *bl = NULL;
    if (nd)
        bl = &nd->bl;

    script_pushint(st, map->getcell(m, bl, x, y, type));

    return true;
}

BUILDIN(setCells)
{
    int m;

    const char *mapname = script_getstr(st, 2);
    int x1 = script_getnum(st, 3);
    int y1 = script_getnum(st, 4);
    int x2 = script_getnum(st, 5);
    int y2 = script_getnum(st, 6);
    int mask = script_getnum(st, 7);
    const char *name = script_getstr(st, 8);

    if ((m = map->mapname2mapid(mapname)) < 0)
        return true; // Invalid Map

    emap_iwall_set2(m, 0, x1, y1, x2, y2, mask, name);

    return true;
}

BUILDIN(delCells)
{
    const char *name = script_getstr(st,2);
    map->iwall_remove(name);
    return true;
}

BUILDIN(setMount)
{
    getSD()
    int mount = script_getnum(st, 2);
    pc_setglobalreg(sd, mountScriptId, mount);
    status_calc_pc(sd, SCO_NONE);
    send_pc_info(&sd->bl, &sd->bl, AREA);
    return true;
}

BUILDIN(setSkin)
{
    if (!st->oid)
        return false;

    getSD()

    const char *skin = script_getstr(st, 2);
    send_pc_skin(sd->fd, st->oid, skin);
    return true;
}

BUILDIN(initCraft)
{
    getSDReturn(-1)

    int var = str_to_craftvar(sd, script_getstr(st, 2));
    script_pushint(st, var);
    return true;
}

BUILDIN(dumpCraft)
{
    getSD()

    craft_dump(sd, script_getnum(st, 2));
    return true;
}

BUILDIN(deleteCraft)
{
    getSD()

    craft_delete(script_getnum(st, 2));
    return true;
}

BUILDIN(getCraftSlotId)
{
    getSDReturn(0)

    const struct craft_slot *crslot = craft_get_slot(script_getnum(st, 2),
        script_getnum(st, 3));
    if (!crslot)
        return false;
    const int len = VECTOR_LENGTH(crslot->items);
    if (len > 0)
    {
        struct item_pair *pair = &VECTOR_INDEX(crslot->items, 0);
        const int invIndex = pair->index;
        const int item_id = sd->status.inventory[invIndex].nameid;
        script_pushint(st, item_id);
    }
    else
    {
        script_pushint(st, 0);
    }
    return true;
}

BUILDIN(getCraftSlotAmount)
{
    getSDReturn(0)

    const struct craft_slot *crslot = craft_get_slot(script_getnum(st, 2),
        script_getnum(st, 3));
    if (!crslot)
        return false;
    const int len = VECTOR_LENGTH(crslot->items);
    if (len > 0)
    {
        int slot;
        int amount = 0;
        for (slot = 0; slot < len; slot ++)
        {
            struct item_pair *pair = &VECTOR_INDEX(crslot->items, slot);
            const int invIndex = pair->index;
            const int item_id = sd->status.inventory[invIndex].nameid;
            if (item_id > 0)
            {
                const int item_amount = sd->status.inventory[invIndex].amount;
                if (item_amount > 0)
                    amount += pair->amount;
            }
        }
        script_pushint(st, amount);
    }
    else
    {
        script_pushint(st, 0);
    }
    return true;
}

BUILDIN(validateCraft)
{
    getSDReturn(0)
    const bool valid = craft_validate(sd, script_getnum(st, 2));
    script_pushint(st, valid ? 1 : 0);
    return true;
}

BUILDIN(findCraftEntry)
{
    getSDReturn(-1)
    const int id = craft_find_entry(sd,
        script_getnum(st, 2),
        script_getnum(st, 3));
    script_pushint(st, id);
    return true;
}

BUILDIN(useCraft)
{
    getSD()
    return craft_use(sd, script_getnum(st, 2));
}

BUILDIN(getCraftCode)
{
    getSDReturn(-1)
    script_pushint(st, craft_get_entry_code(sd, script_getnum(st, 2)));
    return true;
}

BUILDIN(getInvIndexLink)
{
    getSDReturnS("")

    int index = script_getnum (st, 2);

    if (index < 0 || index >= MAX_INVENTORY)
    {
        script_pushstr(st, "");
        return false;
    }

    const int item_id = sd->status.inventory[index].nameid;
    const struct item_data *i_data = NULL;
    if (item_id)
        i_data = itemdb->search(item_id);
    char *const item_name = (char *) aCalloc (1000, sizeof (char));

    if (sd)
    {
        int version = 0;
        struct SessionExt *data = session_get_bysd(sd);
        if (data)
            version = data->clientVersion;

        if (i_data && version >= 7)
        {
            const struct item *const item = &sd->status.inventory[index];
            if (item->card[0] == CARD0_PET ||
                item->card[0] == CARD0_FORGE ||
                item->card[0] == CARD0_CREATE)
            {
                sprintf(item_name, "[@@%u|@@]", (unsigned)i_data->nameid);
            }
            else
            {
                sprintf(item_name, "[@@%u", (unsigned)i_data->nameid);
                int f;
                for (f = 0; f < 4 && item->card[f]; f ++)
                {
                    char buf[100];
                    sprintf(buf, ",%u", (unsigned)item->card[f]);
                    strcat(item_name, buf);
                }
                strcat(item_name, "|@@]");
            }
        }
        else if (i_data)
        {
            sprintf(item_name, "[@@%u|%s@@]", (unsigned)i_data->nameid, lang_pctrans (i_data->jname, sd));
        }
        else if (item_id > 0)
        {
            sprintf(item_name, "[@@%u|Unknown Item@@]", (unsigned)item_id);
        }
        else
        {
            sprintf(item_name, "[Unknown Item]");
        }
    }
    else
    {
        if (i_data)
            sprintf(item_name, "[%s]", lang_pctrans (i_data->jname, sd));
        else
            sprintf(item_name, "[Unknown Item]");
    }

    script_pushstr(st, item_name);

    return true;
}

/*==========================================
 * Shows an emoticon on top of the player/npc
 * emotion emotion#, <target: 0 - NPC, 1 - PC>, <NPC/PC name>
 *------------------------------------------*/
//Optional second parameter added by [Skotlex]
BUILDIN(emotion)
{
    int player = 0;

    int type = script_getnum(st, 2);

    if (script_hasdata(st, 3))
        player = script_getnum(st, 3);

    if (player != 0)
    {
        struct map_session_data *sd = NULL;
        if (script_hasdata(st, 4))
            sd = script->nick2sd(st, script_getstr(st, 4));
        else
            sd = script->rid2sd(st);
        if (sd != NULL)
            clif->emotion(&sd->bl, type);
    }
    else if (script_hasdata(st, 4))
    {
        struct npc_data *nd = npc->name2id(script_getstr(st, 4));
        if (nd != NULL)
            clif->emotion(&nd->bl,type);
    }
    else
    {
        clif->emotion(map->id2bl(st->oid), type);
    }
    return true;
}

BUILDIN(setLook)
{
    const int type = script_getnum(st, 2);
    const int val = script_getnum(st, 3);

    struct map_session_data *sd = script->rid2sd(st);
    if (sd == NULL)
        return true;

    pc->changelook(sd, type, val);
    send_changelook2(sd, &sd->bl, sd->bl.id, type, val, 0, NULL, 0, AREA);
    return true;
}
