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
#include "../../../map/battle.h"
#include "../../../map/npc.h"

#include "map/data/npcd.h"
#include "map/struct/npcdext.h"

struct NpcdExt *npcd_get(struct npc_data *nd)
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
    data->areaSize = AREA_SIZE;
    return data;
}
