// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mapindex.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"

#include "ecommon/config.h"

const char *default_map = NULL;
int default_x = 0;
int default_y = 0;

void config_default_map(const char *key __attribute__ ((unused)),
                        const char *val)
{
    if (mapindex->default_map != NULL &&
        strcmp(mapindex->default_map, MAP_DEFAULT) == 0)
    {
        aFree(mapindex->default_map);
    }
    mapindex->default_map = aStrdup(val);
}

void config_default_x(const char *key __attribute__ ((unused)),
                      const char *val)
{
    mapindex->default_x = atoi(val);
}

void config_default_y(const char *key __attribute__ ((unused)),
                      const char *val)
{
    mapindex->default_y = atoi(val);
}

void common_config_final(void)
{
}
