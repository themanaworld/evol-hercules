// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/malloc.h"
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

        nd = (TBL_NPC *) map->id2bl (st->oid);
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
    const char *str;
    char *msg;
    TBL_NPC *nd = NULL;

    getSD();

    if (script_hasdata(st, 3))
    {
        nd = npc->name2id (script_getstr(st, 2));
        str = script_getstr(st, 3);
    }
    else
    {
        nd = (TBL_NPC *) map->id2bl (st->oid);
        str = script_getstr(st, 2);
    }

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
        msg = (char*)lang_pctrans (nd->name, sd);
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

BUILDIN(getItemLink)
{
    struct item_data *i_data;
    char *item_name;
    int  item_id = 0;

    if (script_isstringtype(st, 2))
    {
        i_data = itemdb->search_name (script_getstr(st, 2));
    }
    else
    {
        item_id = script_getnum (st, 2);
        i_data = itemdb->search (item_id);
    }

    item_name = (char *) aCalloc (100, sizeof (char));
    TBL_PC *sd = script->rid2sd(st);

    if (sd)
    {
        int version = 0;
        struct SessionExt *data = session_get_bysd(sd);
        if (data)
            version = data->clientVersion;

        if (i_data && version >= 7)
            sprintf(item_name, "[@@%u|@@]", (unsigned)i_data->nameid);
        else if (i_data)
            sprintf(item_name, "[@@%u|%s@@]", (unsigned)i_data->nameid, lang_pctrans (i_data->jname, sd));
        else if (item_id > 0)
            sprintf(item_name, "[@@%u|Unknown Item@@]", (unsigned)item_id);
        else
            sprintf(item_name, "[Unknown Item]");
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

BUILDIN(requestLang)
{
    getSD();
    struct script_data* data;
    int64 uid;
    const char* name;

    data = script_getdata(st, 2);
    if (!data_isreference(data))
    {
        ShowError("script:requestlang: not a variable\n");
        script->reportsrc(st);
        st->state = END;
        return false;
    }
    uid = reference_getuid(data);
    name = reference_getname(data);

    if (is_string_variable(name))
    {
        ShowError("script:requestlang: not a variable\n");
        script->reportsrc(st);
        return false;
    }

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
        script->set_reg(st, sd, uid, name, (void*)h64BPTRSIZE(lng), script_getref(st,2));
        st->state = RUN;
    }
    return true;
}

BUILDIN(requestItem)
{
    getSD();
    struct script_data* data;
    int64 uid;
    const char* name;

    data = script_getdata(st, 2);
    if (!data_isreference(data))
    {
        ShowError("script:requestitem: not a variable\n");
        script->reportsrc(st);
        st->state = END;
        return false;
    }
    uid = reference_getuid(data);
    name = reference_getname(data);

    if (is_string_variable(name))
    {
        ShowError("script:requestitem: not a variable\n");
        script->reportsrc(st);
        return false;
    }

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
            script->reportsrc(st);
            return false;
        }

        if (sscanf (sd->npc_str, "%5d", &item) < 1)
        {
            ShowWarning("input data is not item id\n");
            script->reportsrc(st);
            return false;
        }

        script->set_reg(st, sd, uid, name, (void*)h64BPTRSIZE(item), script_getref(st,2));
        st->state = RUN;
    }
    return true;
}

BUILDIN(requestItems)
{
    getSD();
    struct script_data* data;
    int64 uid;
    const char* name;

    data = script_getdata(st, 2);
    if (!data_isreference(data))
    {
        ShowError("script:requestitem: not a variable\n");
        script->reportsrc(st);
        st->state = END;
        return false;
    }
    uid = reference_getuid(data);
    name = reference_getname(data);

    if (!is_string_variable(name))
    {
        ShowWarning("parameter is not variable\n");
        script->reportsrc(st);
        return false;
    }

    int count = 1;

    if (script_hasdata(st, 3))
    {
        count = script_getnum(st, 3);
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
            script->reportsrc(st);
            return false;
        }

        script->set_reg(st, sd, uid, name, (void*)sd->npc_str, script_getref(st, 2));
        st->state = RUN;
    }
    return true;
}

BUILDIN(requestItemIndex)
{
    getSessionData(client);
    struct script_data* data;
    int64 uid;
    const char* name;

    data = script_getdata(st, 2);
    if (!data_isreference(data))
    {
        ShowError("script:requestitem: not a variable\n");
        script->reportsrc(st);
        st->state = END;
        return false;
    }
    uid = reference_getuid(data);
    name = reference_getname(data);

    if (is_string_variable(name))
    {
        ShowError("script:requestitemindex: not a variable\n");
        script->reportsrc(st);
        return false;
    }

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

        int item = 0;

        if (!sd->npc_str)
        {
            ShowWarning("npc string not found\n");
            script->reportsrc(st);
            return false;
        }

        if (sscanf (sd->npc_str, "%5d", &item) < 1)
        {
            ShowWarning("input data is not item id\n");
            script->reportsrc(st);
            return false;
        }

        script->set_reg(st, sd, uid, name, (void*)h64BPTRSIZE(item), script_getref(st,2));
        st->state = RUN;
    }
    return true;
}

BUILDIN(requestItemsIndex)
{
    getSessionData(client);
    struct script_data* data;
    int64 uid;
    const char* name;

    data = script_getdata(st, 2);
    if (!data_isreference(data))
    {
        ShowError("script:requestitem: not a variable\n");
        script->reportsrc(st);
        st->state = END;
        return false;
    }
    uid = reference_getuid(data);
    name = reference_getname(data);

    if (!is_string_variable(name))
    {
        ShowWarning("parameter is not variable\n");
        script->reportsrc(st);
        return false;
    }

    int count = 1;

    if (script_hasdata(st, 3))
    {
        count = script_getnum(st, 3);
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
            ShowWarning("npc string not found\n");
            script->reportsrc(st);
            return false;
        }

        script->set_reg(st, sd, uid, name, (void*)sd->npc_str, script_getref(st, 2));
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

        nd = (TBL_NPC *) map->id2bl (st->oid);
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

BUILDIN(countItemColor)
{
    int nameid, i;
    int count = 0;
    struct item_data* id = NULL;
    getSD();

    if (script_isstringtype(st, 2))
    {
        // item name
        id = itemdb->search_name(script_getstr(st, 2));
    }
    else
    {
        // item id
        id = itemdb->exists(script_getnum(st, 2));
    }

    if (id == NULL)
    {
        ShowError("buildin_countitem: Invalid item '%s'.\n", script_getstr(st,2));  // returns string, regardless of what it was
        script->reportsrc(st);
        script_pushint(st,0);
        return false;
    }

    nameid = id->nameid;

    for(i = 0; i < MAX_INVENTORY; i++)
    {
        if(sd->status.inventory[i].nameid == nameid)
            count += sd->status.inventory[i].amount;
    }

    script_pushint(st, count);
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

enum setmount_type
{
    SETMOUNT_TYPE_NONE         = 0,
    SETMOUNT_TYPE_PECO         = 1,
    SETMOUNT_TYPE_WUG          = 2,
    SETMOUNT_TYPE_MADO         = 3,
    SETMOUNT_TYPE_DRAGON_GREEN = 4,
    SETMOUNT_TYPE_DRAGON_BROWN = 5,
    SETMOUNT_TYPE_DRAGON_GRAY  = 6,
    SETMOUNT_TYPE_DRAGON_BLUE  = 7,
    SETMOUNT_TYPE_DRAGON_RED   = 8,
    SETMOUNT_TYPE_MAX,
    SETMOUNT_TYPE_DRAGON       = SETMOUNT_TYPE_DRAGON_GREEN,
};

BUILDIN(setMount)
{
    int flag = SETMOUNT_TYPE_NONE;
    TBL_PC* sd = script->rid2sd(st);

    if (sd == NULL)
        return true;  // no player attached, report source

    if (script_hasdata(st, 2))
        flag = script_getnum(st, 2);

    // Sanity checks and auto-detection
    if (flag >= SETMOUNT_TYPE_DRAGON_GREEN && flag <= SETMOUNT_TYPE_DRAGON_RED)
    {
        if (pc->checkskill(sd, RK_DRAGONTRAINING))
        {
            // Rune Knight (Dragon)
            unsigned int option;
            option = ( flag == SETMOUNT_TYPE_DRAGON_GREEN ? OPTION_DRAGON1 :
                       flag == SETMOUNT_TYPE_DRAGON_BROWN ? OPTION_DRAGON2 :
                       flag == SETMOUNT_TYPE_DRAGON_GRAY ? OPTION_DRAGON3 :
                       flag == SETMOUNT_TYPE_DRAGON_BLUE ? OPTION_DRAGON4 :
                       flag == SETMOUNT_TYPE_DRAGON_RED ? OPTION_DRAGON5 :
                       OPTION_DRAGON1); // default value
            pc->setridingdragon(sd, option);
        }
    }
    else if (flag == SETMOUNT_TYPE_WUG)
    {
        // Ranger (Warg)
        if (pc->checkskill(sd, RA_WUGRIDER))
            pc->setridingwug(sd, true);
    }
    else if (flag == SETMOUNT_TYPE_MADO)
    {
        // Mechanic (Mado Gear)
        if (pc->checkskill(sd, NC_MADOLICENCE))
            pc->setmadogear(sd, true);
    }
    else if (flag == SETMOUNT_TYPE_PECO)
    {
        // Knight / Crusader (Peco Peco)
        if (pc->checkskill(sd, KN_RIDING))
            pc->setridingpeco(sd, true);
    }
    else if (flag == SETMOUNT_TYPE_NONE && pc_hasmount(sd))
    {
        if (pc_isridingdragon(sd))
        {
            pc->setridingdragon(sd, 0);
        }
        if (pc_isridingwug(sd))
        {
            pc->setridingwug(sd, false);
        }
        if (pc_ismadogear(sd))
        {
            pc->setmadogear(sd, false);
        }
        if (pc_isridingpeco(sd))
        {
            pc->setridingpeco(sd, false);
        }
    }

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

        nd = (TBL_NPC *) map->id2bl (st->oid);
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

        nd = (TBL_NPC *) map->id2bl (st->oid);
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
    struct npc_data *nd = (struct npc_data *)map->id2bl(st->oid);
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
        ShowWarning("checknpccell: Attempted to run on unexsitent map '%s', type %d, x/y %d,%d\n", script_getstr(st, 2), type, x, y);
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

    emap_iwall_set2(m, x1, y1, x2, y2, mask, name);

    return true;
}

BUILDIN(delCells)
{
    const char *name = script_getstr(st,2);
    map->iwall_remove(name);
    return true;
}
