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
#include "../../../map/clif.h"
#include "../../../map/npc.h"
#include "../../../map/pc.h"
#include "../../../map/script.h"

#include "map/script.h"
#include "map/scriptdefines.h"
#include "map/send.h"
#include "map/session.h"
#include "map/sessionext.h"

BUILDIN(l)
{
    // for now not translate and not use format parameters
    script_pushstr(st, aStrdup(script_getstr(st, 2)));
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
    getSD();

    char *str;
    char *msg;
    struct npc_data *nd = NULL;

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

    msg = nd->name;
// +++ after restore lang support need translate here
/*
    if (sd)
        msg = (char*)lang_pctrans (nd->name, sd);
    else
        msg = nd->name;
*/
    if (strlen(str) + strlen(msg) > 450)
        return false;

    if (nd)
    {
        char message[500];
        strcpy (message, msg);
        strcat (message, " : ");
        strcat (message, str);
        send_local_message (sd, &(nd->bl), message);
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
    struct script_data *data;
    int  item_id = 0;

    if (script_isstringtype(st, 2))
    {
        i_data = itemdb->searchname (script_getstr(st, 2));
    }
    else
    {
        item_id = script_getnum (st, 2);
        i_data = itemdb->search (item_id);
    }

    item_name = (char *) aCalloc (100, sizeof (char));
    TBL_PC *sd = script->rid2sd(st)

    if (sd)
    {
        if (i_data)
// +++ after restore lang support need translate here
//            sprintf(item_name, "[@@%u|%s@@]", (unsigned)i_data->nameid, lang_pctrans (i_data->jname, sd));
            sprintf(item_name, "[@@%u|%s@@]", (unsigned)i_data->nameid, i_data->jname);
        else if (item_id > 0)
            sprintf(item_name, "[@@%u|Unknown Item@@]", (unsigned)item_id);
        else
            sprintf(item_name, "[Unknown Item]");
    }
    else
    {
        if (i_data)
// +++ after restore lang support need translate here
//            sprintf(item_name, "[%s]", lang_pctrans (i_data->jname, sd));
            sprintf(item_name, "[%s]", lang_pctrans (i_data->jname, sd));
        else
            sprintf(item_name, "[Unknown Item]");
    }

    script_pushstr(st, item_name);

    return true;
}
