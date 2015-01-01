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
#include "../../../map/unit.h"

#include "map/script.h"
#include "map/clif.h"
#include "map/lang.h"
#include "map/scriptdefines.h"
#include "map/send.h"
#include "map/data/mapd.h"
#include "map/data/npcd.h"
#include "map/data/session.h"
#include "map/struct/mapdext.h"
#include "map/struct/npcdext.h"
#include "map/struct/sessionext.h"
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

        // send item request
        send_npccommand(script->rid2sd(st), st->oid, 10);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;

        int item = 0;

        if (!sd->npc_str)
            return false;

        if (sscanf (sd->npc_str, "%5d", &item) < 1)
            return false;

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
        script->reportdata(data);
        st->state = END;
        return false;
    }
    uid = reference_getuid(data);
    name = reference_getname(data);

    if (!is_string_variable(name))
        return false;

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
        send_npccommand2(script->rid2sd (st), st->oid, 10, count, 0, 0);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;

        if (!sd->npc_str)
            return false;

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

BUILDIN(setNpcDir)
{
    int newdir;
    struct npc_data *nd = 0;

    if (script_hasdata(st, 3))
    {
        nd = npc->name2id (script_getstr(st, 2));
        newdir = script_getnum(st, 3);
    }
    else if (script_hasdata(st, 2))
    {
        if (!st->oid)
            return false;

        nd = (struct npc_data *) map->id2bl (st->oid);
        newdir = script_getnum(st, 2);
    }
    if (!nd)
        return false;

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

    TBL_PC* sd = script->rid2sd(st);
    if (!sd)
        return true;

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
        return true;
    const int m = map->mapname2mapid(mapName);
    if (m < 0)
        return false;
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
        return true;
    }
    const int m = map->mapname2mapid(mapName);
    if (m < 0)
    {
        script_pushint(st, 0);
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
        return true;
    const int m = map->mapname2mapid(mapName);
    if (m < 0)
        return false;
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
        return true;
    const int m = map->mapname2mapid(mapName);
    if (m < 0)
        return false;
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
    struct npc_data *nd = NULL;
    int sex = 0;
    if (script_hasdata(st, 3))
    {
        nd = npc->name2id (script_getstr(st, 2));
        sex = script_getnum(st, 3);
    }
    else if (script_hasdata(st, 2))
    {
        sex = script_getnum(st, 2);
    }
    else
    {
        return false;
    }

    if (!nd && !st->oid)
    {
        return false;
    }

    if (!nd)
        nd = (struct npc_data *) map->id2bl(st->oid);

    if (!nd || !nd->vd)
    {
        script_pushint(st, -1);
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
    if (!music || !mapName)
        return 0;
    const int m = map->mapname2mapid(mapName);
    if (m < 0)
        return false;

    send_changemusic_brodcast(m, music);
    return true;
}

BUILDIN(setNpcDialogTitle)
{
    const char *const name = script_getstr(st, 2);
    if (!name)
        return false;
    struct map_session_data *sd = script->rid2sd (st);
    if (!sd)
        return false;

    send_changenpc_title(sd, st->oid, name);
    return true;
}

BUILDIN(getMapName)
{
    TBL_PC *sd = script->rid2sd(st);
    if (!sd || sd->bl.m == -1)
    {
        script_pushstr(st, aStrdup(""));
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
        return false;

    nameid = script_getnum(st, 2);
    if((item_data = itemdb->exists(nameid)) == NULL)
        return false;
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
