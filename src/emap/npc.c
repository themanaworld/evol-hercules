// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/map.h"
#include "map/npc.h"
#include "map/pc.h"

#include "emap/data/mapd.h"
#include "emap/data/npcd.h"
#include "emap/struct/mapdext.h"
#include "emap/struct/npcdext.h"
#include "emap/npc.h"

void enpc_parse_unknown_mapflag(const char *name, const char *w3, const char *w4, const char* start,
                                const char* buffer, const char* filepath, int *retval)
{
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
        ShowError("npc_parse_mapflag: unrecognized mapflag '%s' in file '%s', line '%d'.\n", w3, filepath, strline(buffer,start-buffer));
        if (retval)
            *retval = EXIT_FAILURE;
    }
    hookStop();
}

int enpc_buysellsel(TBL_PC* sd, int *id, int *type)
{
    TBL_NPC *nd;

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

    if (*type == 0 && nd->subtype == SCRIPT && nd->u.scr.shop && nd->u.scr.shop->type == NST_MARKET)
    {
        clif->npc_market_open(sd, nd);
        sd->npc_shopid = nd->bl.id;
        hookStop();
        return 0;
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

bool enpc_db_checkid(int *idPtr)
{
    const int id = *idPtr;
    hookStop();

    if (id == HIDDEN_WARP_CLASS || id == INVISIBLE_CLASS) // Special IDs not included in the valid ranges
        return true;
    if (id >= 45 && id < MAX_NPC_CLASS) // Second subrange
        return true;
    if (id >= MAX_NPC_CLASS2_START && id < MAX_NPC_CLASS2_END) // Second range
        return true;
    // Anything else is invalid
    return false;
}
