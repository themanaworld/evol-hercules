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
#include "../../../map/itemdb.h"
#include "../../../map/pc.h"

#include "map/pc.h"
#include "map/session.h"
#include "map/sessionext.h"

int langScriptId;

int epc_readparam_pre(struct map_session_data* sd, int *type)
{
    if (*type == Const_ClientVersion)
    {
        hookStop();
        struct SessionExt *data = session_get_bysd(sd);
        if (!data)
            return 0;
        return data->clientVersion;
    }
    return 0;
}

int epc_setregistry(struct map_session_data *sd, int64 *reg, int *val)
{
    if (*reg == langScriptId)
    {
        struct SessionExt *data = session_get_bysd(sd);
        if (!data)
            return 0;

        data->language = *val;
    }

    return 0;
}

#define equipPos(mask, field, lookf) \
    if (pos & mask) \
    { \
        hookStop(); \
        if (id) \
            sd->status.field = id->look; \
        else \
            sd->status.field = 0; \
        clif->changelook(&sd->bl, lookf, sd->status.field); \
    }

#define equipPos2(mask, lookf) \
    if (pos & mask) \
    { \
        hookStop(); \
        if (id) \
            clif->changelook(&sd->bl, lookf, id->look); \
        else \
            clif->changelook(&sd->bl, lookf, 0); \
    }

void epc_equipitem_pos(struct map_session_data *sd, struct item_data *id, int *posPtr)
{
    int pos = *posPtr;

    equipPos(EQP_HEAD_LOW, head_bottom, LOOK_HEAD_BOTTOM);
    equipPos(EQP_HEAD_TOP, head_top, LOOK_HEAD_TOP);
    equipPos(EQP_HEAD_MID, head_mid, LOOK_HEAD_MID);
    equipPos(EQP_GARMENT, robe, LOOK_ROBE);
    //skip EQP_ARMOR
    equipPos2(EQP_SHOES, LOOK_SHOES);
    equipPos2(EQP_COSTUME_HEAD_TOP, 13);
    equipPos2(EQP_COSTUME_HEAD_MID, 14);
    equipPos2(EQP_COSTUME_HEAD_LOW, 15);
    equipPos2(EQP_COSTUME_GARMENT, 16);
    //skipping SHADOW slots
}

#undef equipPos
#undef equipPos2

#define unequipPos(mask, field, lookf) \
    if (pos & mask) \
    { \
        hookStop(); \
        sd->status.field = 0; \
        clif->changelook(&sd->bl, lookf, sd->status.field); \
    }

#define unequipPos2(mask, lookf) \
    if (pos & mask) \
    { \
        hookStop(); \
        clif->changelook(&sd->bl, lookf, 0); \
    }

void epc_unequipitem_pos(struct map_session_data *sd, int *nPtr, int *posPtr)
{
    int pos = *posPtr;
    int n = *nPtr;

    unequipPos(EQP_HEAD_LOW, head_bottom, LOOK_HEAD_BOTTOM);
    unequipPos(EQP_HEAD_TOP, head_top, LOOK_HEAD_TOP);
    unequipPos(EQP_HEAD_MID, head_mid, LOOK_HEAD_MID);
    unequipPos(EQP_GARMENT, robe, LOOK_ROBE);
    //skip EQP_ARMOR
    unequipPos2(EQP_SHOES, LOOK_SHOES);
    unequipPos2(EQP_COSTUME_HEAD_TOP, 13);
    unequipPos2(EQP_COSTUME_HEAD_MID, 14);
    unequipPos2(EQP_COSTUME_HEAD_LOW, 15);
    unequipPos2(EQP_COSTUME_GARMENT, 16);
    //skipping SHADOW slots
}
