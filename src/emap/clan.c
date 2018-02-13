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
#include "map/clan.h"
#include "map/pc.h"

#include "emap/send.h"

bool eclan_join_post(bool retVal,
                     struct map_session_data *sd,
                     int clan_id __attribute__ ((unused)))
{
    if (retVal == true && sd != NULL)
    {
        send_pc_info(&sd->bl, &sd->bl, AREA);
    }
    return retVal;
}

bool eclan_leave_post(bool retVal,
                      struct map_session_data *sd,
                      bool first __attribute__ ((unused)))
{
    if (retVal == true && sd != NULL)
    {
        send_pc_info(&sd->bl, &sd->bl, AREA);
    }
    return retVal;
}
