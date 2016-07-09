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

void skilld_init(void)
{
    for (int f = 0; f < MAX_SKILL_DB; f ++)
    {
        for (int d = 0; d < SKILLD_MAXMISCEFFECTS; d ++)
        {
            skilld_arr[f].miscEffects[d] = -1;
        }
    }
}

struct SkilldExt *skilld_get(const int idx)
{
    Assert_retr(NULL, idx >= 0 && idx < MAX_SKILL_DB);
    return &skilld_arr[idx];
}

struct SkilldExt *skilld_get_id(const int skill_id)
{
    Assert_retr(NULL, skill_id >= 0 && skill_id < MAX_SKILL_ID);
    return &skilld_arr[skill->get_index(skill_id)];
}

int skilld_get_misceffect(const int skill_id,
                          const int effect_idx)
{
    Assert_retr(-1, skill_id >= 0 && skill_id < MAX_SKILL_ID);
    Assert_retr(-1, effect_idx >= 0 && effect_idx < SKILLD_MAXMISCEFFECTS);
    return skilld_arr[skill->get_index(skill_id)].miscEffects[effect_idx];
}
