// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/conf.h"
#include "common/malloc.h"
#include "common/mapindex.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/sysinfo.h"
#include "common/timer.h"

void interfaces_init_common(void)
{
}

void setDefaultMap(void)
{
    mapindex->default_map = aStrdup("000-1");
    mapindex->default_x = 80;
    mapindex->default_y = 109;
}

void commonClean(void)
{
    aFree(mapindex->default_map);
    mapindex->default_map = NULL;
}