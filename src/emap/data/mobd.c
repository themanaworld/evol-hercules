// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/battle.h"
#include "map/mob.h"

#include "emap/data/mobd.h"
#include "emap/struct/mobdext.h"

struct MobdExt *mobd_get(struct mob_db *md)
{
    struct MobdExt *data = getFromMOBDB(md, 0);
    if (!data)
    {
        data = mobd_create();
        addToMOBDB(md, data, 0, true);
    }
    return data;
}

struct MobdExt *mobd_create(void)
{
    struct MobdExt *data = NULL;
    CREATE(data, struct MobdExt, 1);
    if (!data)
        return NULL;
    data->walkMask = 0;
    return data;
}
