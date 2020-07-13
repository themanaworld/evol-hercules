// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2016 Evol developers

#include "common/hercules.h"
#include "map/clif.h"
#include "map/map.h"
#include "map/status.h"
#include "map/skill.h"

void eskill_physical_shield(struct block_list *src,
                            struct block_list *bl,
                            uint16 skill_id,
                            uint16 skill_lv)
{
    int val1, val2, time, matk;
    enum sc_type type;

    type = status->skill2sc(skill_id);
    matk = status->get_matk(src, 2);
    val1 = skill_lv * 10 + matk / 20;     // DEF bonus
    val2 = skill_lv * 5 + matk / 25;      // ASPD penalty
    time = skill->get_time(skill_id, skill_lv);  // doesn't depends on matk
    clif->skill_nodamage(src, bl, skill_id, skill_lv,
                         sc_start2(src, bl, type, 100, val1, val2, time));
}
