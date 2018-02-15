// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/utils.h"
#include "common/timer.h"
#include "map/chat.h"
#include "map/chrif.h"
#include "map/instance.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/script.h"
#include "map/quest.h"

#include "emap/clif.h"
#include "emap/craft.h"
#include "emap/lang.h"
#include "emap/map.h"
#include "emap/hashtable.h"
#include "emap/scriptdefines.h"
#include "emap/send.h"
#include "emap/data/bgd.h"
#include "emap/data/mapd.h"
#include "emap/data/session.h"
#include "emap/struct/bgdext.h"
#include "emap/struct/mapdext.h"
#include "emap/struct/sessionext.h"
#include "emap/utils/formatutils.h"

extern int mountScriptId;
extern char global_npc_str[1001];

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

BUILDIN(closeClientDialog)
{
    getSD();
    send_npccommand(sd, st->oid, 14);
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
        if (i_data)
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
            sprintf(item_name, "[%s]", i_data->jname);
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
        if (*global_npc_str)
            lng = lang_getId(global_npc_str);
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
        st->state = RUN;

        int item = 0;

        if (!*global_npc_str)
        {
            ShowWarning("npc string not found\n");
            script_pushint(st, 0);
            script->reportsrc(st);
            return false;
        }

        if (sscanf (global_npc_str, "%5d", &item) < 1)
        {
            ShowWarning("input data is not item id\n");
            script_pushint(st, 0);
            script->reportsrc(st);
            return false;
        }

        script_pushint(st, item);
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
        st->state = RUN;

        if (!*global_npc_str)
        {
            ShowWarning("npc string not found\n");
            script_pushstr(st, aStrdup("0,0"));
            script->reportsrc(st);
            return false;
        }

        script_pushstr(st, aStrdup(global_npc_str));
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
        if (client)
            send_npccommand(sd, st->oid, 11);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;
        st->state = RUN;

        int item = -1;

        if (!*global_npc_str)
        {
            script_pushint(st, -1);
            ShowWarning("npc string not found\n");
            script->reportsrc(st);
            return false;
        }

        if (sscanf (global_npc_str, "%5d", &item) < 1)
        {
            script_pushint(st, -1);
            ShowWarning("input data is not item id\n");
            script->reportsrc(st);
            return false;
        }

        script_pushint(st, item);
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
        if (client)
            send_npccommand2(sd, st->oid, 11, count, 0, 0);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;
        st->state = RUN;

        if (!*global_npc_str)
        {
            script_pushstr(st, aStrdup("-1"));
            ShowWarning("npc string not found\n");
            script->reportsrc(st);
            return false;
        }

        script_pushstr(st, aStrdup(global_npc_str));
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
        if (client)
            send_npccommand2(sd, st->oid, 12, count, 0, 0);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;
        st->state = RUN;

        if (!*global_npc_str)
        {
            ShowWarning("npc string not found\n");
            script->reportsrc(st);
            script_pushstr(st, aStrdup(""));
            return false;
        }

        script_pushstr(st, aStrdup(global_npc_str));
    }
    return true;
}

BUILDIN(setq)
{
    int i;
    getSD();

    int quest_id = script_getnum(st, 2);

    if (quest->check(sd, quest_id, HAVEQUEST) < 0)
        quest->add(sd, quest_id, 0);
    ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
    if (i == sd->num_quests)
    {
        ShowError("Quest with id=%d not found\n", quest_id);
        script->reportsrc(st);
        return false;
    }

    sd->quest_log[i].count[0] = script_getnum(st, 3);
    if (script_hasdata(st, 4))
        sd->quest_log[i].count[1] = script_getnum(st, 4);
    if (script_hasdata(st, 5))
        sd->quest_log[i].count[2] = script_getnum(st, 5);
    if (script_hasdata(st, 6))
        sd->quest_log[i].time = script_getnum(st, 6);
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

BUILDIN(getq2)
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
    script_pushint(st, sd->quest_log[i].count[1]);
    return true;
}

BUILDIN(getq3)
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
    script_pushint(st, sd->quest_log[i].count[2]);
    return true;
}

BUILDIN(getqTime)
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
    script_pushint(st, sd->quest_log[i].time);
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
    npc->enable(nd->exname, 1);

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

    if (!nd)
    {
        ShowWarning("npc not found\n");
        script->reportsrc(st);
        return false;
    }

    clif->clearunit_area(&nd->bl, CLR_OUTSIGHT);
    nd->vd.sex = sex;
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
                pc->addfame(sd, RANKTYPE_BLACKSMITH, 1); // Success to refine to +10 a lv1 weapon you forged = +1 fame point
                break;
            case 2:
                pc->addfame(sd, RANKTYPE_BLACKSMITH, 25); // Success to refine to +10 a lv2 weapon you forged = +25 fame point
                break;
            case 3:
                pc->addfame(sd, RANKTYPE_BLACKSMITH, 1000); // Success to refine to +10 a lv3 weapon you forged = +1000 fame point
                break;
        }
    }

    return true;
}

BUILDIN(successRemoveCardsIndex)
{
    int i = -1, c, cardflag = 0;

    getSD()
    getInventoryIndex(2)

    if (sd->status.inventory[n].nameid <= 0 || sd->status.inventory[n].amount <= 0)
        return false;

    i = n;

    if (itemdb_isspecial(sd->status.inventory[i].card[0]))
        return true;

    for (c = sd->inventory_data[i]->slot - 1; c >= 0; --c)
    {
        if (sd->status.inventory[i].card[c] && itemdb_type(sd->status.inventory[i].card[c]) == IT_CARD)
        {   // extract this card from the item
            int flag;
            struct item item_tmp;
            memset(&item_tmp, 0, sizeof(item_tmp));
            cardflag = 1;
            item_tmp.nameid   = sd->status.inventory[i].card[c];
            item_tmp.identify = 1;
            sd->status.inventory[i].card[c] = 0;

            if ((flag = pc->additem(sd, &item_tmp, 1, LOG_TYPE_SCRIPT)))
            {
                // get back the cart in inventory
                clif->additem(sd, 0, 0, flag);
                map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
            }
        }
    }

    if (cardflag == 1)
    {
        pc->unequipitem(sd, i, PCUNEQUIPITEM_FORCE);
        clif->delitem(sd, i, 1, DELITEM_MATERIALCHANGE);
        clif->additem(sd, i, 1, 0);
        pc->equipitem(sd, i, sd->status.inventory[i].equip);
        clif->misceffect(&sd->bl,3);
    }
    return true;
}

/// <type>=0 : will destroy both the item and the cards.
/// <type>=1 : will keep the item, but destroy the cards.
/// <type>=2 : will keep the cards, but destroy the item.
/// <type>=? : will just display the failure effect.
BUILDIN(failedRemoveCardsIndex)
{
    int i = -1, c, cardflag = 0;

    getSD()
    getInventoryIndex(2)

    if (sd->status.inventory[n].nameid <= 0 || sd->status.inventory[n].amount <= 0)
        return false;

    i = n;

    int typefail = script_getnum(st, 3);

    if (itemdb_isspecial(sd->status.inventory[i].card[0]))
        return true;

    for (c = sd->inventory_data[i]->slot - 1; c >= 0; --c)
    {
        if (sd->status.inventory[i].card[c] && itemdb_type(sd->status.inventory[i].card[c]) == IT_CARD)
        {
            cardflag = 1;
            sd->status.inventory[i].card[c] = 0;

            if (typefail == 2)
            {   // add cards to inventory, clear
                int flag;
                struct item item_tmp;

                memset(&item_tmp, 0, sizeof(item_tmp));

                item_tmp.nameid   = sd->status.inventory[i].card[c];
                item_tmp.identify = 1;

                if ((flag = pc->additem(sd, &item_tmp, 1, LOG_TYPE_SCRIPT)))
                {
                    clif->additem(sd, 0, 0, flag);
                    map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
                }
            }
        }
    }

    if (cardflag == 1)
    {
        if (typefail == 0 || typefail == 2)
        {
            // destroy the item
            pc->delitem(sd, i, 1, 0, DELITEM_FAILREFINE, LOG_TYPE_SCRIPT);
        }
        else if (typefail == 1)
        {
            pc->unequipitem(sd, i, PCUNEQUIPITEM_FORCE);
            clif->delitem(sd, i, 1, DELITEM_MATERIALCHANGE);
            clif->additem(sd, i, 1, 0);
            pc->equipitem(sd, i, sd->status.inventory[i].equip);
        }
        clif->misceffect(&sd->bl, 2);
    }

    return true;
}

BUILDIN(getCardByIndex)
{
    getSD()
    getInventoryIndex(2)

    if (sd->status.inventory[n].nameid <= 0 || sd->status.inventory[n].amount <= 0)
    {
        script_pushint(st, 0);
        return false;
    }

    const int c = script_getnum(st, 3);
    if (c < 0 || c >= MAX_SLOTS)
    {
        script_pushint(st, 0);
        return false;
    }

    if (itemdb_isspecial(sd->status.inventory[n].card[0]))
    {
        script_pushint(st, 0);
        return true;
    }

    const int card = sd->status.inventory[n].card[c];
    if (card && itemdb_type(card) == IT_CARD)
    {
        script_pushint(st, card);
    }
    else
    {
        script_pushint(st, 0);
    }

    return true;
}

BUILDIN(removeCardByIndex)
{
    getSD()
    getInventoryIndex(2)

    if (sd->status.inventory[n].nameid <= 0 || sd->status.inventory[n].amount <= 0)
    {
        ShowWarning("zero amount\n");
        script_pushint(st, -1);
        return false;
    }

    if (sd->status.inventory[n].equip)
    {
        ShowWarning("item equipped\n");
        script_pushint(st, -1);
        return false;
    }

    const int c = script_getnum(st, 3);
    if (c < 0 || c >= MAX_SLOTS)
    {
        ShowWarning("wrong slot id\n");
        script_pushint(st, -1);
        return false;
    }

    const int amount = sd->status.inventory[n].amount;
    clif->delitem(sd, n, amount, DELITEM_FAILREFINE);
    sd->status.inventory[n].card[c] = 0;
    clif->additem(sd, n, amount, 0);
    status_calc_pc(sd, SCO_NONE);

    script_pushint(st, 0);
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
    nd->vd.dead_sit = 2;
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
    nd->vd.dead_sit = 0;
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
        nd->vd.dead_sit = 0;
        script_pushint(st, unit->walktoxy(&nd->bl,x,y,0));
        return true;
    }
    else
    {
        ShowWarning("npc not found\n");
        script->reportsrc(st);
        script_pushint(st, 0);
        return false;
    }
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
    send_pc_own_flags(&sd->bl);
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
        if (i_data)
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
            sprintf(item_name, "[%s]", i_data->jname);
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

BUILDIN(setFakeCells)
{
    const int x1 = script_getnum(st, 2);
    const int y1 = script_getnum(st, 3);
    int x2 = x1, y2 = y1, block_type = 0;

    if (script_hasdata(st, 6))
    {
        if (script_isinttype(st, 5) && script_isinttype(st, 6))
        {
            x2 = script_getnum(st, 4);
            y2 = script_getnum(st, 5);
            block_type = script_getnum(st, 6);
        }
        else
        {
            ShowError("all arguments must be of type int");
            return false;
        }
    }
    else
    {
        block_type = script_getnum(st, 4);
    }

    getSD();
    send_setwall_single(sd->fd, sd->bl.m, 0, x1, y1, x2, y2, block_type);
    return true;
}

#define checkHashTableExists(id) \
    if (!htreg->hashtable_exists(id)) \
    { \
        ShowError("%s: hashtable with id=%d does not exist\n", __func__, (int)id); \
        script_pushint(st, 0); \
        return false; \
    }

BUILDIN(htNew)
{
    int64 id = htreg->new_hashtable();
    script_pushint(st, id);
    return true;
}

BUILDIN(htGet)
{
    int64 id = script_getnum(st, 2);
    checkHashTableExists(id);

    struct DBData defval_s;
    struct DBData *defval = &defval_s;
    const char * key = script_getstr(st, 3);

    if (script_hasdata(st, 4))
    {
        if (script_isstringtype(st, 4))
        {
            defval->type = DB_DATA_PTR;
            defval->u.ptr = (void*)script_getstr(st, 4);
        }
        else if (script_isinttype(st, 4))
        {
            defval->type = DB_DATA_INT;
            defval->u.i = script_getnum(st, 4);
        }
        else
        {
            ShowError("usage: htget(<id>, <strkey>[ ,<defval>])\n");
            return false;
        }
    }
    else
    {
        defval = NULL;
    }

    const struct DBData *result = htreg->hashtable_getvalue(id, key, defval);
    if (result)
    {
        switch(result->type)
        {
            case DB_DATA_INT:
            case DB_DATA_UINT:
                script_pushint(st, result->u.i);
                break;
            case DB_DATA_PTR:
                script_pushstrcopy(st, result->u.ptr);
                break;
        }
    }
    else
    {
        script_pushint(st, 0);
    }

    return true;
}

BUILDIN(htPut)
{
    int64 id = script_getnum(st, 2);
    checkHashTableExists(id);

    struct DBData value;
    const char * key = script_getstr(st, 3);

    if (script_isstringtype(st, 4))
    {
        value.type = DB_DATA_PTR;
        value.u.ptr = (void*)aStrdup(script_getstr(st, 4));
    }
    else if (script_isinttype(st, 4))
    {
        value.type = DB_DATA_INT;
        value.u.i = script_getnum(st, 4);
    }
    else
    {
        ShowError("usage: htput(<id>, <strkey>, <newval>)\n");
        return false;
    }

    htreg->hashtable_setvalue(id, key, value);
    return true;
}

BUILDIN(htClear)
{
    int64 id = script_getnum(st, 2);
    checkHashTableExists(id);

    htreg->clear_hashtable(id);
    return true;
}

BUILDIN(htDelete)
{
    int64 id = script_getnum(st, 2);
    checkHashTableExists(id);

    htreg->destroy_hashtable(id);
    return true;
}

BUILDIN(htSize)
{
    int64 id = script_getnum(st, 2);
    checkHashTableExists(id);

    script_pushint(st, htreg->hashtable_size(id));
    return true;
}

BUILDIN(htIterator)
{
    int64 id = script_getnum(st, 2);
    checkHashTableExists(id);

    script_pushint(st, htreg->create_iterator(id));
    return true;
}

#undef checkHashTableExists

#define checkHtIteratorExists(id) \
    if (!htreg->iterator_exists(id)) \
    { \
        ShowError("%s: htIterator with id=%d does not exist\n", __func__, (int)id); \
        script_pushint(st, 0); \
        return false; \
    }

BUILDIN(htiNextKey)
{
    int64 id = script_getnum(st, 2);
    checkHtIteratorExists(id);

    const char * key = htreg->iterator_nextkey(id);
    if (key)
        script_pushstrcopy(st, key);
    else
        script_pushstrcopy(st, "");
    return true;
}

BUILDIN(htiCheck)
{
    int64 id = script_getnum(st, 2);
    checkHtIteratorExists(id);

    if (htreg->iterator_check(id))
        script_pushint(st, 1);
    else
        script_pushint(st, 0);

    return true;
}

BUILDIN(htiDelete)
{
    int64 id = script_getnum(st, 2);
    checkHtIteratorExists(id);

    htreg->destroy_iterator(id);
    return true;
}

#undef checkHtIteratorExists

BUILDIN(getLabel)
{
    script_pushint(st, script_getnum(st, 2));
    return true;
}

BUILDIN(toLabel)
{
    script_pushlabel(st, script_getnum(st, 2));
    return true;
}

// replace default input with bigger text buffer
/// Get input from the player.
/// For numeric inputs the value is capped to the range [min,max]. Returns 1 if
/// the value was higher than 'max', -1 if lower than 'min' and 0 otherwise.
/// For string inputs it returns 1 if the string was longer than 'max', -1 is
/// shorter than 'min' and 0 otherwise.
///
/// input(<var>{,<min>{,<max>}}) -> <int>
BUILDIN(input)
{
    struct script_data* data;
    int64 uid;
    const char* name;
    int min;
    int max;
    struct map_session_data *sd = script->rid2sd(st);
    if (sd == NULL)
        return true;

    data = script_getdata(st, 2);
    if (!data_isreference(data))
    {
        ShowError("script:input: not a variable\n");
        script->reportdata(data);
        st->state = END;
        return false;
    }
    uid = reference_getuid(data);
    name = reference_getname(data);
    min = (script_hasdata(st,3) ? script_getnum(st,3) : script->config.input_min_value);
    max = (script_hasdata(st,4) ? script_getnum(st,4) : script->config.input_max_value);

#ifdef SECURE_NPCTIMEOUT
    sd->npc_idle_type = NPCT_WAIT;
#endif

    if (!sd->state.menu_or_input)
    {
        // first invocation, display npc input box
        sd->state.menu_or_input = 1;
        st->state = RERUNLINE;
        if (is_string_variable(name))
            clif->scriptinputstr(sd, st->oid);
        else
            clif->scriptinput(sd, st->oid);
    }
    else
    {
        // take received text/value and store it in the designated variable
        sd->state.menu_or_input = 0;
        if (is_string_variable(name))
        {
            int len = (int)strlen(global_npc_str);
            script->set_reg(st, sd, uid, name, global_npc_str, script_getref(st,2));
            script_pushint(st, (len > max ? 1 : len < min ? -1 : 0));
        }
        else
        {
            int amount = sd->npc_amount;
            script->set_reg(st, sd, uid, name, (const void *)h64BPTRSIZE(cap_value(amount,min,max)), script_getref(st,2));
            script_pushint(st, (amount > max ? 1 : amount < min ? -1 : 0));
        }
        st->state = RUN;
    }
    return true;
}

BUILDIN(slide)
{
    getSDReturn(false);
    const int x = script_getnum(st,2);
    const int y = script_getnum(st,3);
    const int16 m = sd->bl.m;

    if (x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys)
    {
        ShowError("slide: attempt to place player %s (%d:%d) on invalid coordinates (%d,%d)\n", sd->status.name, sd->status.account_id, sd->status.char_id, x, y);
        script->reportsrc(st);
        return false;
    }

    if (map->getcell(m, &sd->bl, x, y, CELL_CHKNOPASS) && pc_get_group_level(sd) < battle->bc->gm_ignore_warpable_area)
    {
        return false;
    }

    clif->slide(&sd->bl, x, y);
    unit->movepos(&sd->bl, x, y, 1, 0);
    return true;
}

BUILDIN(getItemOptionIdByIndex)
{
    getSD()
    getInventoryIndex(2)

    if (sd->status.inventory[n].nameid <= 0 || sd->status.inventory[n].amount <= 0)
    {
        script_pushint(st, 0);
        return false;
    }

    const int optIndex = script_getnum(st, 3);
    if (optIndex < 0 || optIndex >= MAX_ITEM_OPTIONS)
    {
        script_pushint(st, 0);
        return false;
    }

    const int optType = sd->status.inventory[n].option[optIndex].index;
    script_pushint(st, optType);

    return true;
}

BUILDIN(getItemOptionValueByIndex)
{
    getSD()
    getInventoryIndex(2)

    if (sd->status.inventory[n].nameid <= 0 || sd->status.inventory[n].amount <= 0)
    {
        script_pushint(st, 0);
        return false;
    }

    const int optIndex = script_getnum(st, 3);
    if (optIndex < 0 || optIndex >= MAX_ITEM_OPTIONS)
    {
        script_pushint(st, 0);
        return false;
    }

    const int optValue = sd->status.inventory[n].option[optIndex].value;
    script_pushint(st, optValue);

    return true;
}

BUILDIN(getItemOptionParamByIndex)
{
    getSD()
    getInventoryIndex(2)

    if (sd->status.inventory[n].nameid <= 0 || sd->status.inventory[n].amount <= 0)
    {
        script_pushint(st, 0);
        return false;
    }

    const int optIndex = script_getnum(st, 3);
    if (optIndex < 0 || optIndex >= MAX_ITEM_OPTIONS)
    {
        script_pushint(st, 0);
        return false;
    }

    const int optParam = sd->status.inventory[n].option[optIndex].param;
    script_pushint(st, optParam);

    return true;
}

BUILDIN(setItemOptionByIndex)
{
    getSD()
    getInventoryIndex(2)

    if (sd->status.inventory[n].nameid <= 0 || sd->status.inventory[n].amount <= 0)
        return false;

    if (itemdb->isstackable2(sd->inventory_data[n]))
        return false;

    // not allow change equipped items
    if (sd->status.inventory[n].equip != 0)
        return false;

    const int optIndex = script_getnum(st, 3);
    if (optIndex < 0 || optIndex >= MAX_ITEM_OPTIONS)
    {
        ShowError("Wrong optIndex in setitemoptionbyindex\n");
        return false;
    }
    int optType = 0;
    int optValue = 0;
    if (script_hasdata(st, 5))
    {
        optType = script_getnum(st, 4);
        optValue = script_getnum(st, 5);
    }
    else
    {
        optType = sd->status.inventory[n].option[optIndex].index;
        optValue = script_getnum(st, 4);
    }

    if (optType != 0 && itemdb->option_exists(optType) == NULL)
    {
        ShowError("Wrong optType in setitemoptionbyindex\n");
        return false;
    }

    clif->delitem(sd, n, 1, DELITEM_MATERIALCHANGE);
    logs->pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->status.inventory[n], sd->inventory_data[n]);

    sd->status.inventory[n].option[optIndex].index = optType;
    sd->status.inventory[n].option[optIndex].value = optValue;

    clif->additem(sd, n, 1, 0);
    logs->pick_pc(sd, LOG_TYPE_SCRIPT, 1, &sd->status.inventory[n], sd->inventory_data[n]);

    return true;
}

BUILDIN(isInstance)
{
    const int instance_id = script_getnum(st, 2);
    script_pushint(st, instance->valid(instance_id) ? 1 : 0);
    return true;
}
