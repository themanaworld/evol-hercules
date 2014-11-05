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

BUILDIN(l)
{
    // for now not translate and not use format parameters
    script_pushstr(st, strdup(script_getstr(st, 2)));
    return true;
}
