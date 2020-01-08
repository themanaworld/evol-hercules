// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"

HPExport struct HPMi_interface HPMi_s;
HPExport struct HPMi_interface *HPMi;
HPExport void *(*import_symbol) (char *name, unsigned int pID);

#include "common/conf.h"
#include "common/memmgr.h"
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

#define checkVar(name, value) \
    if (name != value) \
    { \
        ShowError(#name" wrong value. Found %d but must be %d.\n", \
            name, \
            value); \
    }

void common_online(void)
{
    checkVar(MAX_SKILL_DB, 1532);
    checkVar(MAX_SKILL_ID, 20022);
    checkVar(SC_MAX, 658);
    checkVar(SI_MAX, 971);
    checkVar(OLD_MAX_SKILL_DB, 1510);
    checkVar(MAX_EVOL_SKILLS, 22);
    checkVar(EVOL_FIRST_SKILL, 20000);
    checkVar(MAX_SKILL_TREE, 110);
    checkVar(BASE_GUILD_SIZE, 100);
}
