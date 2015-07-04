// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/map.h"

#include "emap/data/mapd.h"
#include "emap/struct/mapdext.h"

struct MapdExt *mapd_get(int m)
{
    struct map_data *md = &map->list[m];
    struct MapdExt *data = getFromMAPD(md, 0);
    if (!data)
    {
        data = mapd_create();
        addToMAPD(md, data, 0, true);
    }
    return data;
}

struct MapdExt *mapd_create(void)
{
    struct MapdExt *data = NULL;
    CREATE(data, struct MapdExt, 1);
    if (!data)
        return NULL;
    data->mask = 1;
    data->invisible = false;
    data->flag.nopve = 0;
    return data;
}
