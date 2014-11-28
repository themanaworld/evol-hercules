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
#include "../../../map/pc.h"

#include "map/session.h"
#include "map/struct/sessionext.h"

struct SessionExt *session_get(int fd)
{
    struct SessionExt *data = getFromSession(session[fd], 0);
    if (!data)
    {
        data = session_create();
        addToSession(session[fd], data, 0, true);
    }
    return data;
}

struct SessionExt *session_get_bysd(struct map_session_data *sd)
{
    if (!sd)
        return NULL;

    return session_get(sd->fd);
}

struct SessionExt *session_create(void)
{
    struct SessionExt *data = NULL;
    CREATE(data, struct SessionExt, 1);
    if (!data)
        return NULL;
    data->clientVersion = 0;
    data->language = 0;
    return data;
}
