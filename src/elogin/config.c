// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"

#include "elogin/config.h"

char *update_server = NULL;

void config_update_server(const char *key __attribute__ ((unused)),
                          const char *val)
{
    if (update_server != NULL)
        aFree(update_server);
    update_server = aStrdup(val);
}

void config_final(void)
{
    if (update_server != NULL)
        aFree(update_server);
}
