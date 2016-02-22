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

bool warn_missing_translation = false;

void config_warn_missing_translation(const char *key __attribute__ ((unused)),
                                     const char *val)
{
    warn_missing_translation = config_switch(val) ? true : false;
}
