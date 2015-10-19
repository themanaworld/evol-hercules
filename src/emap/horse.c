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
#include "map/map.h"
#include "map/pc.h"

#include "emap/data/session.h"
#include "emap/struct/sessionext.h"

void horse_add_bonus(TBL_PC *sd)
{
    struct SessionExt *data = session_get_bysd(sd);
    if (!data || data->mount == 0)
        return;

    struct status_data *bstatus = &sd->base_status;

    bstatus->aspd_rate += 50 - 10 * pc->checkskill(sd, KN_CAVALIERMASTERY);

    if (pc->checkskill(sd, KN_RIDING) > 0)
        sd->max_weight += 10000;
}

unsigned short horse_add_speed_bonus(TBL_PC *sd, unsigned short val)
{
    if (sd)
    {
        struct SessionExt *data = session_get_bysd(sd);
        if (!data || data->mount == 0)
            return val;

        val -= 25;
    }
    return val;
}
