// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/map.h"
#include "map/npc.h"
#include "map/pc.h"

#include "plugins/HPMHooking.h"

#include "emap/data/mapd.h"
#include "emap/data/npcd.h"
#include "emap/struct/mapdext.h"
#include "emap/struct/npcdext.h"
#include "emap/map.h"
#include "emap/npc.h"

void enpc_parse_unknown_mapflag_pre(const char **namePtr,
                                    const char **w3Ptr,
                                    const char **w4Ptr,
                                    const char **startPtr,
                                    const char **bufferPtr,
                                    const char **filepathPtr,
                                    int **retvalPtr)
{
    const char *name = *namePtr;
    const char *w3 = *w3Ptr;
    const char *w4 = *w4Ptr;
    int *retval = *retvalPtr;

    if (!strcmpi(w3, "invisible"))
    {
        int16 m = map->mapname2mapid(name);
        struct MapdExt *data = mapd_get(m);
        if (data)
            data->invisible = true;
    }
    else if (!strcmpi(w3, "mask"))
    {
        int16 m = map->mapname2mapid(name);
        struct MapdExt *data = mapd_get(m);
        if (data)
            data->mask = atoi(w4);
    }
    else if (!strcmpi(w3, "nopve"))
    {
        int16 m = map->mapname2mapid(name);
        struct MapdExt *data = mapd_get(m);
        if (data)
            data->flag.nopve = 1;
    }
    else
    {
        ShowError("npc_parse_mapflag: unrecognized mapflag '%s' in file '%s', line '%d'.\n",
            w3,
            *filepathPtr,
            strline(*bufferPtr, *startPtr - *bufferPtr));
        if (retval)
            *retval = EXIT_FAILURE;
    }
    hookStop();
}

int enpc_buysellsel_pre(TBL_PC **sdPtr,
                        int *id,
                        int *type)
{
    TBL_NPC *nd;
    TBL_PC *sd = *sdPtr;

    if (!sd)
        return 1;

    if ((nd = npc->checknear(sd, map->id2bl(*id))) == NULL)
    {
        hookStop();
        return 1;
    }

    if (nd->option & OPTION_INVISIBLE) // can't buy if npc is not visible (hack?)
    {
        hookStop();
        return 1;
    }

    if (*type == 0 && nd->subtype == SCRIPT && nd->u.scr.shop)
    {
        if (nd->u.scr.shop->type == NST_MARKET)
        {
            clif->npc_market_open(sd, nd);
            sd->npc_shopid = nd->bl.id;
            hookStop();
            return 0;
        }
        else if (nd->u.scr.shop->type == NST_BARTER)
        {
            clif->npc_barter_open(sd, nd);
            sd->npc_shopid = nd->bl.id;
            hookStop();
            return 0;
        }
        else if (nd->u.scr.shop->type == NST_CUSTOM)
        {
            clif->cashshop_show(sd, nd);
            sd->npc_shopid = nd->bl.id;
            hookStop();
            return 0;
        }
    }

    if (nd->subtype != SHOP && !(nd->subtype == SCRIPT && nd->u.scr.shop && nd->u.scr.shop->items))
    {
        if (nd->subtype == SCRIPT)
            ShowError("npc_buysellsel: trader '%s' has no shop list!\n", nd->exname);
        else
            ShowError("npc_buysellsel: no such shop npc %d (%s)\n", *id, nd->exname);

        if (sd->npc_id == *id)
            sd->npc_id = 0;
        hookStop();
        return 1;
    }

    if (nd->class_ < 0 && !sd->state.callshop)
    {  // not called through a script and is not a visible NPC so an invalid call
        hookStop();
        return 1;
    }

    // reset the callshop state for future calls
    sd->state.callshop = 0;
    sd->npc_shopid = *id;

    if (*type == 0)
        clif->buylist(sd, nd);
    else
        clif->selllist(sd);

    hookStop();
    return 0;
}

bool enpc_db_checkid_pre(const int *idPtr)
{
    const int id = *idPtr;
    hookStop();

    if (id == HIDDEN_WARP_CLASS || id == INVISIBLE_CLASS) // Special IDs not included in the valid ranges
        return true;
    if (id >= 45 && id < MAX_NPC_CLASS) // Second subrange
        return true;
    if (id >= MAX_NPC_CLASS2_START && id < MAX_NPC_CLASS2_END) // Second range
        return true;
    if (pc->db_checkid(id))
        return true;
    // Anything else is invalid
    return false;
}

bool enpc_duplicate_script_sub_pre(struct npc_data **ndPtr,
                                   const struct npc_data **sndPtr,
                                   int *xsPtr,
                                   int *ysPtr,
                                   int *optionsPtr __attribute__ ((unused)))
{
    struct npc_data *nd = *ndPtr;
    const struct npc_data *snd = *sndPtr;

    hookStop();
    nullpo_retr(false, nd);
    nullpo_retr(false, snd);

    int xs = *xsPtr;
    int ys = *ysPtr;

    int i;
    bool retval = true;

    ++npc->npc_script;
    nd->u.scr.xs = xs;
    nd->u.scr.ys = ys;
    nd->u.scr.script = snd->u.scr.script;
    nd->u.scr.label_list = snd->u.scr.label_list;
    nd->u.scr.label_list_num = snd->u.scr.label_list_num;
    nd->u.scr.shop = snd->u.scr.shop;
    nd->u.scr.trader = snd->u.scr.trader;

    struct script_code *code;
    CREATE(code, struct script_code, 1);

    const int sz = VECTOR_LENGTH(snd->u.scr.script->script_buf);
    VECTOR_INIT(code->script_buf);
    VECTOR_ENSURE(code->script_buf, sz , 1);
    VECTOR_PUSHARRAY(code->script_buf, VECTOR_DATA(snd->u.scr.script->script_buf), sz);

    code->local.vars = NULL;
    code->local.arrays = NULL;
    nd->u.scr.script = code;

    enpc_set_var_num(nd, ".parent", snd->bl.id);

    //add the npc to its location
    npc->add_to_location(nd);

    // Loop through labels to export them as necessary
    for (i = 0; i < nd->u.scr.label_list_num; i++)
    {
        if (npc->event_export(nd, i))
        {
            ShowWarning("npc_parse_duplicate: duplicate event %s::%s in file '%s'.\n",
                nd->exname, nd->u.scr.label_list[i].name, nd->path);
            retval = false;
        }
        npc->timerevent_export(nd, i);
    }

    nd->u.scr.timerid = INVALID_TIMER;

// run OnInit always
//    if (options&NPO_ONINIT)
    {
        // From npc_parse_script
        char evname[EVENT_NAME_LENGTH];
        struct event_data *ev;

        snprintf(evname, ARRAYLENGTH(evname), "%s::OnInit", nd->exname);

        if ((ev = (struct event_data*)strdb_get(npc->ev_db, evname)) != NULL)
        {
            //Execute OnInit
            script->run_npc(nd->u.scr.script,ev->pos,0,nd->bl.id);
        }
    }
    hookStop();
    return retval;
}

void enpc_set_var_num(TBL_NPC *const npc,
                      const char *var,
                      const int val)
{
    const int num = (int)reference_uid(script->add_str(var), 0);
    if (!npc->u.scr.script->local.vars)
        npc->u.scr.script->local.vars = i64db_alloc(DB_OPT_RELEASE_DATA);
    i64db_iput(npc->u.scr.script->local.vars, num, val);
}

int enpc_get_var_num(const TBL_NPC *const npc,
                     const char *var)
{
    const int num = (int)reference_uid(script->add_str(var), 0);
    if (npc->u.scr.script->local.vars)
    {
        return i64db_iget(npc->u.scr.script->local.vars, num);
    }
    else
    {
        return 0;
    }
}

int enpc_unload_pre(struct npc_data** ndPtr,
                    bool *singlePtr __attribute__ ((unused)))
{
    struct npc_data *nd = *ndPtr;
    nullpo_ret(nd);
    map_alwaysVisible_delete(&nd->bl);
    if (nd->subtype == SCRIPT)
    {
        if (nd->src_id != 0)
        {
            if (nd->u.scr.script)
            {
                script->free_code(nd->u.scr.script);
                nd->u.scr.script = NULL;
            }
/*
// this need to clean if we copy this structs too.
            if (nd->u.scr.label_list)
            {
                aFree(nd->u.scr.label_list);
                nd->u.scr.label_list = NULL;
                nd->u.scr.label_list_num = 0;
            }
            if (nd->u.scr.shop)
            {
                if(nd->u.scr.shop->item)
                    aFree(nd->u.scr.shop->item);
                aFree(nd->u.scr.shop);
            }
*/
        }
    }
    return 0;
}
