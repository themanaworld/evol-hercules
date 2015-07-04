// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/db.h"
#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/battle.h"
#include "map/itemdb.h"
#include "map/map.h"
#include "map/pc.h"

bool email_invalid_operation(struct map_session_data *sd)
{
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
