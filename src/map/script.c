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
#include "../../../map/clif.h"
#include "../../../map/pc.h"
#include "../../../map/script.h"

#include "map/session.h"
#include "map/sessionext.h"

#define getData(def) \
    if (!st->rid) \
    { \
        script_pushint(st, 0); \
        return true; \
    } \
    TBL_PC *sd = script->rid2sd(st); \
    if (!sd) \
    { \
        script_pushint(st, 0); \
        return true; \
    } \
    struct SessionExt *data = session_get(sd->fd)

BUILDIN(l)
{
    // for now not translate and not use format parameters
    script_pushstr(st, aStrdup(script_getstr(st, 2)));
    return true;
}

BUILDIN(getClientVersion)
{
    getData(0);
    script_pushint(st, data->clientVersion);
}

BUILDIN(getLang)
{
    getData(0);
    script_pushint(st, data->language);
}
