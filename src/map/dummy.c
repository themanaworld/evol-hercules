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

//#include "../../../common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

void showNotImplimented()
{
    ShowWarning("Not implimented\n");
}

BUILDIN(dummy)
{
    showNotImplimented();
    return true;
}

BUILDIN(dummyStr)
{
    showNotImplimented();
    script_pushconststr(st, "");
    return true;
}

BUILDIN(dummyInt)
{
    showNotImplimented();
    script_pushint(st, 0);
    return true;
}
