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

#include "map/parse.h"
#include "map/data/session.h"
#include "map/struct/sessionext.h"

void map_parse_version(int fd)
{
//    struct map_session_data* sd = (struct map_session_data*)session[fd]->session_data;
//    if (!sd)
//        return;

    struct SessionExt *data = session_get(fd);
    data->clientVersion = RFIFOL(fd, 2);
}
