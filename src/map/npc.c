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
#include "../../../map/npc.h"
#include "../../../map/pc.h"

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

    if (bl->m != sd->bl.m ||
        bl->x < sd->bl.x - AREA_SIZE - 1 || bl->x > sd->bl.x + AREA_SIZE + 1 ||
        bl->y < sd->bl.y - AREA_SIZE - 1 || bl->y > sd->bl.y + AREA_SIZE + 1)
    {
        return NULL;
    }

    return nd;
}