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
#include "map/send.h"
#include "map/session.h"
#include "map/sessionext.h"

#define getDataReturn(def) \
    if (!st->rid) \
    { \
        script_pushint(st, def); \
        return true; \
    } \
    TBL_PC *sd = script->rid2sd(st); \
    if (!sd) \
    { \
        script_pushint(st, def); \
        return true; \
    } \
    struct SessionExt *data = session_get(sd->fd)

#define getData() \
    if (!st->rid) \
        return true; \
    TBL_PC *sd = script->rid2sd(st); \
    if (!sd) \
        return true; \
    struct SessionExt *data = session_get(sd->fd)

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
}

BUILDIN(getLang)
{
    getDataReturn(0);
    script_pushint(st, data->language);
}

BUILDIN(setLang)
{
    getData();
    data->language = script_getnum(st, 2);
}

BUILDIN(setCamNpc)
{
    TBL_PC *sd = script->rid2sd(st);
    if (!sd)
        return 1;

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
            return 1;

        nd = (struct npc_data *) map->id2bl (st->oid);
    }
    if (!nd || sd->bl.m != nd->bl.m)
        return 1;

    if (script_hasdata(st, 3) && script_hasdata(st, 4))
    {
        x = script_getnum(st, 3);
        y = script_getnum(st, 4);
    }

    send_npccommand2(script->rid2sd (st), st->oid, 2, nd->bl.id, x, y);

    return 0;
}
