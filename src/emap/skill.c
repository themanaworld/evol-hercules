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
#include "map/pc.h"
#include "map/npc.h"
#include "map/script.h"

#include "emap/skill.h"
#include "emap/skill_const.h"
#include "emap/skill_ground.h"
#include "emap/skill_targeted.h"
#include "emap/status.h"

#include "plugins/HPMHooking.h"

int eskill_get_index_post(int retVal,
                          uint16 skill_id)
{
    if (skill_id >= EVOL_FIRST_SKILL && skill_id < EVOL_FIRST_SKILL + MAX_EVOL_SKILLS)
    {
        // 1478 + skill_id - 20000
        skill_id = OLD_MAX_SKILL_DB + skill_id - EVOL_FIRST_SKILL;
        return skill_id;
    }
    return retVal;
}

int eskill_check_condition_castend_post(int retVal,
                                        TBL_PC* sd,
                                        uint16 skill_id,
                                        uint16 skill_lv)
{
    if (retVal && sd)
    {
        struct linkdb_node **label_linkdb = strdb_get(npc->ev_label_db, "OnSkillInvoke");
        if (label_linkdb == NULL)
            return retVal;

        struct linkdb_node *node = *label_linkdb;
        while (node)
        {
            struct event_data* ev = node->data;
            if (ev)
            {
                pc->setreg(sd, script->add_str("@skillId"), skill_id);
                pc->setreg(sd, script->add_str("@skillLv"), skill_lv);
                script->run(ev->nd->u.scr.script, ev->pos, sd->bl.id, ev->nd->bl.id);
            }
            node = node->next;
        }
    }
    return retVal;
}

bool eskill_castend_nodamage_id_unknown(struct block_list *src,
                                        struct block_list *bl,
                                        uint16 *skill_id,
                                        uint16 *skill_lv,
                                        int64 *tick __attribute__ ((unused)),
                                        int *flag __attribute__ ((unused)))
{
    switch (*skill_id)
    {
        case EVOL_PHYSICAL_SHIELD:
            eskill_physical_shield(src, bl, *skill_id, *skill_lv);
            break;

        default:
            clif->skill_nodamage(src, bl, *skill_id, *skill_lv, 1);
            break;
    }
    map->freeblock_unlock();
    return true;
}

void eskill_additional_effect_unknown(struct block_list* src __attribute__ ((unused)),
                                      struct block_list *bl __attribute__ ((unused)),
                                      uint16 *skill_id __attribute__ ((unused)),
                                      uint16 *skill_lv __attribute__ ((unused)),
                                      int *attack_type __attribute__ ((unused)),
                                      int *dmg_lv __attribute__ ((unused)),
                                      int64 *tick __attribute__ ((unused)))
{
}

void eskill_counter_additional_effect_unknown(struct block_list *src __attribute__ ((unused)),
                                              struct block_list *bl __attribute__ ((unused)),
                                              uint16 *skill_id __attribute__ ((unused)),
                                              uint16 *skill_lv __attribute__ ((unused)),
                                              int *attack_type __attribute__ ((unused)),
                                              int64 *tick __attribute__ ((unused)))
{
}

void eskill_attack_combo2_unknown(int *attack_type __attribute__ ((unused)),
                                  struct block_list *src __attribute__ ((unused)),
                                  struct block_list *dsrc __attribute__ ((unused)),
                                  struct block_list *bl __attribute__ ((unused)),
                                  uint16 *skill_id __attribute__ ((unused)),
                                  uint16 *skill_lv __attribute__ ((unused)),
                                  int64 *tick __attribute__ ((unused)),
                                  int *flag __attribute__ ((unused)),
                                  int *combo __attribute__ ((unused)))
{
}

void eskill_attack_post_unknown(int *attack_type __attribute__ ((unused)),
                                struct block_list *src __attribute__ ((unused)),
                                struct block_list *dsrc __attribute__ ((unused)),
                                struct block_list *bl __attribute__ ((unused)),
                                uint16 *skill_id __attribute__ ((unused)),
                                uint16 *skill_lv __attribute__ ((unused)),
                                int64 *tick __attribute__ ((unused)),
                                int *flag __attribute__ ((unused)))
{
}

void eskill_timerskill_notarget_unknown(int tid __attribute__ ((unused)),
                                        int64 tick __attribute__ ((unused)),
                                        struct block_list *src __attribute__ ((unused)),
                                        struct unit_data *ud __attribute__ ((unused)),
                                        struct skill_timerskill *skl __attribute__ ((unused)))
{
}

void eskill_unitsetting1_unknown(struct block_list *src __attribute__ ((unused)),
                                 uint16 *skill_id __attribute__ ((unused)),
                                 uint16 *skill_lv __attribute__ ((unused)),
                                 int16 *x __attribute__ ((unused)),
                                 int16 *y __attribute__ ((unused)),
                                 int *flag __attribute__ ((unused)),
                                 int *val1 __attribute__ ((unused)),
                                 int *val2 __attribute__ ((unused)),
                                 int *val3 __attribute__ ((unused)))
{
}

void eskill_unit_onplace_unknown(struct skill_unit *src __attribute__ ((unused)),
                                 struct block_list *bl __attribute__ ((unused)),
                                 int64 *tick __attribute__ ((unused)))
{
}

void eskill_check_condition_castend_unknown(struct map_session_data *sd __attribute__ ((unused)),
                                            uint16 *skill_id __attribute__ ((unused)),
                                            uint16 *skill_lv __attribute__ ((unused)))
{
}

void eskill_get_requirement_unknown(struct status_change *sc __attribute__ ((unused)),
                                    struct map_session_data *sd __attribute__ ((unused)),
                                    uint16 *skill_id __attribute__ ((unused)),
                                    uint16 *skill_lv __attribute__ ((unused)),
                                    struct skill_condition *req __attribute__ ((unused)))
{
}

bool eskill_castend_pos2_unknown(struct block_list* src,
                                 int *x,
                                 int *y,
                                 uint16 *skill_id,
                                 uint16 *skill_lv,
                                 int64 *tick,
                                 int *flag)
{
    switch (*skill_id)
    {
        case EVOL_MASS_PROVOKE:
            return eskill_massprovoke_castend(src, x, y, skill_id, skill_lv, tick, flag);
        default:
            ShowWarning("skill_castend_pos2: Unknown skill used:%d\n", *skill_id);
            return true;
    }
}
