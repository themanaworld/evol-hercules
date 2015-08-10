// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"

#include "elogin/config.h"

char *update_server = NULL;

void config_update_server(const char *val)
{
    update_server = aStrdup(val);
}

void config_final(void)
{
    if (update_server)
        aFree(update_server);
}
