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
#include "../../../map/itemdb.h"
#include "../../../map/map.h"
#include "../../../map/npc.h"
#include "../../../map/pc.h"

#include "map/data/mapd.h"
#include "map/data/npcd.h"
#include "map/struct/mapdext.h"
#include "map/struct/npcdext.h"
#include "map/npc.h"

bool eitemdb_is_item_usable(struct item_data *item)
{
    hookStop();
    return item->type == IT_HEALING || item->type == IT_USABLE || item->type == IT_CASH || item->type == IT_PETEGG;
}
