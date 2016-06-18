// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/battle.h"
#include "map/npc.h"

#include "emap/data/npcd.h"
#include "emap/struct/npcdext.h"

struct NpcdExt *npcd_get(TBL_NPC *nd)
{
    struct NpcdExt *data = getFromNPCD(nd, 0);
    if (!data)
    {
        data = npcd_create();
        addToNPCD(nd, data, 0, true);
    }
    return data;
}

struct NpcdExt *npcd_create(void)
{
    struct NpcdExt *data = NULL;
    CREATE(data, struct NpcdExt, 1);
    if (!data)
        return NULL;
    data->init = false;
    data->language = 0;
    data->walkMask = 0;
    return data;
}

void npcd_copy(TBL_NPC *snd,
               TBL_NPC *nd)
{
    const struct NpcdExt *sData = npcd_get(snd);
    struct NpcdExt *dData = npcd_get(nd);
    if (!snd || !nd)
        return;
    dData->init = sData->init;
    dData->language = sData->language;
    dData->walkMask = sData->walkMask;
}
