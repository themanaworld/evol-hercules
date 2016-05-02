// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/db.h"
#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/battle.h"
#include "map/itemdb.h"
#include "map/map.h"
#include "map/pc.h"

#include "plugins/HPMHooking.h"

bool email_invalid_operation_pre(struct map_session_data **sdPtr)
{
    struct map_session_data *sd = *sdPtr;
    if (!sd)
    {
        hookStop();
        return true;
    }

    if (!map->list[sd->bl.m].flag.town)
    {
        ShowWarning("clif->parse_Mail: char '%s' trying to do invalid mail operations.\n", sd->status.name); 
        hookStop();
        return true;
    }
    hookStop();
    return false;
}
