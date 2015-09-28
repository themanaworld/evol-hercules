// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include "common/hercules.h"

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
#include "map/battleground.h"
#include "map/itemdb.h"
#include "map/map.h"
#include "map/pc.h"

#include "emap/data/bgd.h"
#include "emap/data/session.h"
#include "emap/struct/bgdext.h"
#include "emap/struct/sessionext.h"

bool ebg_team_warp(int *bg_idPtr, unsigned short *map_index, short *x, short *y)
{
    int i;
    int bg_id = *bg_idPtr;
    struct battleground_data *bgd = bg->team_search(bg_id);
    if (bgd == NULL)
        return false;
    struct BgdExt *bdata = bgd_get(bgd);
    if (!bdata)
    {
        ShowWarning("bdata empty\n");
        return true;
    }
    for (i = 0; i < MAX_BG_MEMBERS; i++)
    {
        TBL_PC *sd = bgd->members[i].sd;
        if (sd != NULL)
        {
            struct SessionExt *sdata = session_get_bysd(sd);
            if (sdata)
                sdata->teamId = bdata->teamId;
            pc->setpos(bgd->members[i].sd, *map_index, *x, *y, CLR_TELEPORT);
        }
    }
    return true;
}
