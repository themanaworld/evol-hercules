// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../common/db.h"
#include "../../../common/HPMi.h"
#include "../../../common/malloc.h"
#include "../../../common/mmo.h"
#include "../../../common/socket.h"
#include "../../../common/strlib.h"
#include "../../../common/timer.h"
#include "../../../map/pc.h"
#include "../../../map/npc.h"
#include "../../../map/script.h"

int eskill_check_condition_castend_post(int retVal,
                                        struct map_session_data* sd,
                                        uint16 *skill_id,
                                        uint16 *skill_lv)
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
                pc->setreg(sd, script->add_str("@skillId"), *skill_id);
                pc->setreg(sd, script->add_str("@skillLv"), *skill_lv);
                script->run(ev->nd->u.scr.script, ev->pos, sd->bl.id, ev->nd->bl.id);
            }
            node = node->next;
        }
    }
    return retVal;
}
