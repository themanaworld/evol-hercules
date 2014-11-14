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
