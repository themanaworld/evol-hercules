// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/socket.h"
#include "map/map.h"

#include "emap/console.h"
#include "emap/inter.h"

CPCMD(serverExit)
{
    int code = 0;
    if (!line || !*line || sscanf(line, "%5d", &code) < 1)
    {
        ShowError("Wrong parameter\n");
        return;
    }

    send_char_exit(code);

    map->retval = code;
    map->do_shutdown();
}
