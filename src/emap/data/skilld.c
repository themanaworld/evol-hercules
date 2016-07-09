// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/skill.h"

#include "emap/data/skilld.h"
#include "emap/struct/skilldext.h"

struct SkilldExt skilld_arr[MAX_SKILL_DB];

struct SkilldExt *skilld_init(void)
{
    for (int f = 0; f < MAX_SKILL_DB; f ++)
    {
        for (int d = 0; d < SKILLD_MAXMISCEFFECTS; d ++)
        {
            skilld_arr[f].miscEffects[d] = -1;
        }
    }
}

struct SkilldExt *skilld_get(const int skill_idx)
{
    Assert_retr(NULL, skill_idx >= 0 && skill_idx < MAX_SKILL_DB);
    return &skilld_arr[skill_idx];
}
