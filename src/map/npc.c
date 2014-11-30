// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../common/HPMi.h"
#include "../../../common/malloc.h"
#include "../../../common/mmo.h"
#include "../../../common/socket.h"
#include "../../../common/strlib.h"
#include "../../../map/map.h"
#include "../../../map/npc.h"
#include "../../../map/pc.h"

#include "map/data/mapd.h"
#include "map/data/npcd.h"
#include "map/struct/mapdext.h"
#include "map/struct/npcdext.h"
#include "map/npc.h"

struct npc_data* enpc_checknear(struct map_session_data* sd, struct block_list* bl)
{
    struct npc_data *nd;

    hookStop();

    if (!sd)
        return NULL;

    if (bl == NULL)
        return NULL;
    if (bl->type != BL_NPC)
        return NULL;
    nd = (TBL_NPC*)bl;

    if (sd->npc_id == bl->id)
        return nd;

    if (nd->class_ < 0) //Class-less npc, enable click from anywhere.
        return nd;

    const int npcX = bl->x;
    const int npcY = bl->y;
    const int x = sd->bl.x;
    const int y = sd->bl.y;

    if (bl->m != sd->bl.m
        || npcX < x - AREA_SIZE - 1 || npcX > x + AREA_SIZE + 1
        || npcY < y - AREA_SIZE - 1 || npcY > y + AREA_SIZE + 1)
    {
        return NULL;
    }

    struct NpcdExt *data = npcd_get(nd);
    if (data)
    {
        const int size = data->areaSize;
        if (npcX < x - size || npcX > x + size
            || npcY < y - size || npcY > y + size)
        {
            return NULL;
        }
    }

    return nd;
}

void enpc_parse_unknown_mapflag(const char *name, char *w3, char *w4, const char* start,
                                const char* buffer, const char* filepath, int *retval)
{
    hookStop();
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
}
