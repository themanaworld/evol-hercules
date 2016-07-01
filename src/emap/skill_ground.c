// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/nullpo.h"
#include "common/timer.h"
#include "map/mob.h"
#include "map/skill.h"

#include "emap/skill_ground.h"

static int eskill_massprovoke_sub(struct block_list *bl,
                                  va_list ap)
{
    nullpo_ret(bl);

    if (bl->type != BL_MOB)
        return 0;

    struct block_list* src = va_arg(ap, struct block_list*);
    int dist = va_arg(ap, int);
    int *cnt = va_arg(ap, int*);
    struct status_change *tsc = status->get_sc(bl);
    struct mob_data *dstmd = BL_UCAST(BL_MOB, bl);

    if (tsc && tsc->count)
    {
        status_change_end(bl, SC_FREEZE, INVALID_TIMER);
        if (tsc->data[SC_STONE] && tsc->opt1 == OPT1_STONE)
            status_change_end(bl, SC_STONE, INVALID_TIMER);
        status_change_end(bl, SC_SLEEP, INVALID_TIMER);
        status_change_end(bl, SC_TRICKDEAD, INVALID_TIMER);
    }

    if (dstmd && src)
    {
        dstmd->state.provoke_flag = src->id;
        mob->target(dstmd, src, dist);
        (*cnt) ++;
    }

    return 0;
}

bool eskill_massprovoke_castend(struct block_list* src,
                                int *x,
                                int *y,
                                uint16 *skill_id,
                                uint16 *skill_lv,
                                int64 *tick  __attribute__ ((unused)),
                                int *flag  __attribute__ ((unused)))
{
    nullpo_retr(false, src);
    const int r = skill->get_splash(*skill_id, *skill_lv);
    const int dist = skill->get_range2(src, *skill_id, *skill_lv);
    int cnt = 0;
    map->foreachinarea(eskill_massprovoke_sub, src->m, *x - r, *y - r, *x + r, *y + r, BL_MOB,
        src, dist, &cnt);
    if (cnt == 0)
        unit->skillcastcancel(src, 1);
    return false;
}
