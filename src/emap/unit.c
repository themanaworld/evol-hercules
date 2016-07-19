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
#include "common/timer.h"
#include "map/unit.h"
#include "map/map.h"
#include "map/mob.h"
#include "map/pc.h"
#include "map/skill.h"
#include "map/status.h"

#include "plugins/HPMHooking.h"

#include "emap/unit.h"

int eunit_can_move_pre(struct block_list **blPtr)
{
    TBL_PC *sd;
    struct unit_data *ud;
    struct status_change *sc;
    struct block_list *bl = *blPtr;

    if (!bl)
    {
        hookStop();
        return 0;
    }
    ud = unit->bl2ud(bl);
    sc = status->get_sc(bl);
    sd = BL_CAST(BL_PC, bl);

    if (!ud)
    {
        hookStop();
        return 0;
    }

    if (ud->skilltimer != INVALID_TIMER &&
        ud->skill_id != LG_EXEEDBREAK &&
        (!sd ||
        (!pc->checkskill(sd, SA_FREECAST) &&
        (skill->get_inf2(ud->skill_id) & (INF2_GUILD_SKILL | INF2_FREE_CAST_REDUCED | INF2_FREE_CAST_NORMAL)) == 0)))
    {
        hookStop();
        return 0; // prevent moving while casting
    }

    if (DIFF_TICK(ud->canmove_tick, timer->gettick()) > 0)
    {
        hookStop();
        return 0;
    }

    if (sd && (
        sd->state.vending ||
        sd->state.buyingstore ||
        sd->state.blockedmove))
    {
        hookStop();
        return 0; //Can't move
    }

    // Status changes that block movement
    if (sc)
    {
        if (sc->count && (
                sc->data[SC_ANKLESNARE]
            ||  sc->data[SC_AUTOCOUNTER]
            ||  sc->data[SC_TRICKDEAD]
            ||  sc->data[SC_BLADESTOP]
            ||  sc->data[SC_BLADESTOP_WAIT]
            || (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_SELF) // cannot move while gospel is in effect
            || (sc->data[SC_BASILICA] && sc->data[SC_BASILICA]->val4 == bl->id) // Basilica caster cannot move
            ||  sc->data[SC_STOP]
            || sc->data[SC_FALLENEMPIRE]
            ||  sc->data[SC_RG_CCONFINE_M]
            ||  sc->data[SC_RG_CCONFINE_S]
            ||  sc->data[SC_GS_MADNESSCANCEL]
            || (sc->data[SC_GRAVITATION] && sc->data[SC_GRAVITATION]->val3 == BCT_SELF)
            ||  sc->data[SC_WHITEIMPRISON]
            ||  sc->data[SC_ELECTRICSHOCKER]
            ||  sc->data[SC_WUGBITE]
            ||  sc->data[SC_THORNS_TRAP]
            ||  ( sc->data[SC_MAGNETICFIELD] && !sc->data[SC_HOVERING] )
            ||  sc->data[SC__MANHOLE]
            ||  sc->data[SC_CURSEDCIRCLE_ATKER]
            ||  sc->data[SC_CURSEDCIRCLE_TARGET]
            || (sc->data[SC_COLD] && bl->type != BL_MOB)
            ||  sc->data[SC_DEEP_SLEEP]
            || (sc->data[SC_CAMOUFLAGE] && sc->data[SC_CAMOUFLAGE]->val1 < 3 && !(sc->data[SC_CAMOUFLAGE]->val3&1))
            ||  sc->data[SC_MEIKYOUSISUI]
            ||  sc->data[SC_KG_KAGEHUMI]
            ||  sc->data[SC_NEEDLE_OF_PARALYZE]
            ||  sc->data[SC_VACUUM_EXTREME]
            || (sc->data[SC_FEAR] && sc->data[SC_FEAR]->val2 > 0)
            || (sc->data[SC_SPIDERWEB] && sc->data[SC_SPIDERWEB]->val1)
            || (sc->data[SC_CLOAKING] && sc->data[SC_CLOAKING]->val1 < 3 && !(sc->data[SC_CLOAKING]->val4&1)) //Need wall at level 1-2
            || (
                 sc->data[SC_DANCING] && sc->data[SC_DANCING]->val4
                 && (
                       !sc->data[SC_LONGING]
                    || (sc->data[SC_DANCING]->val1&0xFFFF) == CG_MOONLIT
                    || (sc->data[SC_DANCING]->val1&0xFFFF) == CG_HERMODE))))
        {
            hookStop();
            return 0;
        }
        if (sc->opt1 > 0
            && sc->opt1 != OPT1_STONEWAIT
            && sc->opt1 != OPT1_BURNING
            && !(sc->opt1 == OPT1_CRYSTALIZE
            && bl->type == BL_MOB))
        {
            hookStop();
            return 0;
        }

        if ((sc->option & OPTION_HIDE) && (!sd || pc->checkskill(sd, RG_TUNNELDRIVE) <= 0))
        {
            hookStop();
            return 0;
        }
    }

    // Icewall walk block special trapped monster mode
    if(bl->type == BL_MOB)
    {
        TBL_MOB *md = BL_CAST(BL_MOB, bl);
        if(md && ((md->status.mode&MD_BOSS && battle->bc->boss_icewall_walk_block == 1 && map->getcell(bl->m, bl, bl->x, bl->y, CELL_CHKICEWALL))
           || (!(md->status.mode&MD_BOSS) && battle->bc->mob_icewall_walk_block == 1 && map->getcell(bl->m, bl, bl->x, bl->y, CELL_CHKICEWALL))))
        {
            md->walktoxy_fail_count = 1; //Make sure rudeattacked skills are invoked
            hookStop();
            return 0;
        }
    }

    hookStop();
    return 1;
}

int eunit_walktoxy_pre(struct block_list **blPtr  __attribute__ ((unused)),
                       short *x  __attribute__ ((unused)),
                       short *y  __attribute__ ((unused)),
                       int *flagPtr)
{
    struct block_list *bl = *blPtr;
    // reset flag "Search for an unoccupied cell and cancel if none available"
    // this reduce CPU usage and allow mobs to walk on each other.
    if ((*flagPtr)&8)
        *flagPtr = ((*flagPtr) | 8) ^ 8;

    TBL_PC *sd = BL_CAST(BL_PC, bl);
    if (sd && pc_issit(sd))
    {
        pc->setstand(sd);
        skill->sit(sd, 0);
    }

    return 1;
}
