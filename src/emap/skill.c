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

#include "plugins/HPMHooking.h"

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
