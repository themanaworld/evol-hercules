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
        {
            ShowWarning("npc not attached\n");
            script->reportsrc(st);
            return false;
        }

        nd = (struct npc_data *) map->id2bl (st->oid);
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
    {
        ShowWarning("npc not found\n");
        script->reportsrc(st);
        return false;
    }

    if (sd)
        msg = (char*)lang_pctrans (nd->name, sd);
    else
        msg = nd->name;
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
    send_npccommand(script->rid2sd (st), st->oid, 5);
    return true;
}

BUILDIN(shop)
{
    getSD();
    struct npc_data *nd = npc->name2id (script_getstr(st, 2));
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

        // send item request
        send_npccommand(script->rid2sd(st), st->oid, 10);
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
        send_npccommand2(script->rid2sd (st), st->oid, 10, count, 0, 0);
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
    if (!quest->check(sd, quest_id, HAVEQUEST) < 0)
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
    struct npc_data *nd = 0;

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

        nd = (struct npc_data *) map->id2bl (st->oid);
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

    TBL_PC* sd = script->rid2sd(st);
    if (!sd)
    {
        ShowWarning("player not attached\n");
        script->reportsrc(st);
        return false;
    }

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
        ShowWarning("no parameters provided\n");
        script->reportsrc(st);
        return false;
    }

    if (!nd && !st->oid)
    {
        ShowWarning("npc not found\n");
        script->reportsrc(st);
        return false;
    }

    if (!nd)
        nd = (struct npc_data *) map->id2bl(st->oid);

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
    struct map_session_data *sd = script->rid2sd (st);
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
    const int item = va_arg(ap, int);
    int *const amount = va_arg(ap, int *);
    struct flooritem_data *drop = (struct flooritem_data *)bl;

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
