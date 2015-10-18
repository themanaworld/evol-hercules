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
#include "map/battleground.h"

#include "emap/data/bgd.h"
#include "emap/struct/bgdext.h"

struct BgdExt *bgd_get(struct battleground_data *bd)
{
    struct BgdExt *data = getFromBGDATA(bd, 0);
    if (!data)
    {
        data = bgd_create();
        addToBGDATA(bd, data, 0, true);
    }
    return data;
}

struct BgdExt *bgd_create(void)
{
    struct BgdExt *data = NULL;
    CREATE(data, struct BgdExt, 1);
    if (!data)
        return NULL;
    data->teamId = 0;
    return data;
}
