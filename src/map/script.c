// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../common/HPMi.h"
#include "../../../common/malloc.h"
#include "../../../common/mmo.h"
#include "../../../common/socket.h"
#include "../../../common/strlib.h"
#include "../../../map/chrif.h"
#include "../../../map/clif.h"
#include "../../../map/npc.h"
#include "../../../map/pc.h"
#include "../../../map/script.h"
#include "../../../map/quest.h"

#include "map/script.h"
#include "map/clif.h"
#include "map/lang.h"
#include "map/scriptdefines.h"
#include "map/send.h"
#include "map/session.h"
#include "map/sessionext.h"
#include "map/utils/formatutils.h"

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

BUILDIN(getClientVersion)
{
    getDataReturn(0);
    script_pushint(st, data->clientVersion);
    return true;
}

BUILDIN(getLang)
{
    getDataReturn(0);
    script_pushint(st, data->language);
    return true;
}

BUILDIN(setLang)
{
    getData();
    data->language = script_getnum(st, 2);
    return true;
}

BUILDIN(setCamNpc)
{
    getSD();
    struct npc_data *nd = NULL;

    int x = 0;
    int y = 0;

    if (script_hasdata(st, 2))
    {
        nd = npc->name2id (script_getstr(st, 2));
    }
    else
    {
        if (!st->oid)
            return false;

        nd = (struct npc_data *) map->id2bl (st->oid);
    }
    if (!nd || sd->bl.m != nd->bl.m)
        return false;

    if (script_hasdata(st, 3) && script_hasdata(st, 4))
    {
        x = script_getnum(st, 3);
        y = script_getnum(st, 4);
    }

    send_npccommand2(script->rid2sd (st), st->oid, 2, nd->bl.id, x, y);

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
    struct npc_data *nd = NULL;

    getSD();

    if (script_hasdata(st, 3))
    {
        nd = npc->name2id (script_getstr(st, 2));
        str = script_getstr(st, 3);
    }
    else
    {
        nd = (struct npc_data *) map->id2bl (st->oid);
        str = script_getstr(st, 2);
    }

    if (!nd)
        return false;

    if (sd)
        msg = (char*)lang_pctrans (nd->name, sd);
    else
        msg = nd->name;
    if (strlen(str) + strlen(msg) > 450)
        return false;

    if (nd)
    {
        char message[500];
        strcpy (message, msg);
        strcat (message, " : ");
        strcat (message, str);
        send_local_message (sd->fd, &(nd->bl), message);
    }

    return true;
}

BUILDIN(closeDialog)
{
    getSD();
    send_npccommand(script->rid2sd (st), st->oid, 5);
    return true;
}

BUILDIN(shop)
{
    getSD();
    struct npc_data *nd = npc->name2id (script_getstr(st, 2));
    if (!nd)
        return false;

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
        if (i_data)
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
        script->reportdata(data);
        st->state = END;
        return false;
    }
    uid = reference_getuid(data);
    name = reference_getname(data);

    if (is_string_variable(name))
        return false;

    if (!sd->state.menu_or_input)
    {
        // first invocation, display npc input box
        sd->state.menu_or_input = 1;
        st->state = RERUNLINE;

        // send lang request
        send_npccommand(script->rid2sd(st), st->oid, 0);
        clif->scriptinputstr(sd, st->oid);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;

        int lng = -1;
        if (*sd->npc_str)
        {
            lng = lang_getId(sd->npc_str);
        }
        script->set_reg(st, sd, uid, name, (void*)h64BPTRSIZE(lng), script_getref(st,2));
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
    if (!quest->check(sd, quest_id, HAVEQUEST) < 0)
    {
        script_pushint(st, 0);
        return true;
    }
    ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
    if (i == sd->num_quests)
    {
        script_pushint(st, 0);
        return false;
    }
    script_pushint(st, sd->quest_log[i].count[0]);
    return true;
}

BUILDIN(getNpcDir)
{
    struct npc_data *nd = 0;

    if (script_hasdata(st, 2))
    {
        nd = npc->name2id (script_getstr(st, 2));
    }
    if (!nd && !st->oid)
    {
        script_pushint(st, -1);
        return true;
    }

    if (!nd)
        nd = (struct npc_data *) map->id2bl (st->oid);

    if (!nd)
    {
        script_pushint(st, -1);
        return true;
    }

    script_pushint(st, (int)nd->dir);

    return true;
}
