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
#include "login/login.h"

#include "plugins/HPMHooking.h"

#include "elogin/md5calc.h"

bool elogin_check_password_pre(const char **md5key __attribute__ ((unused)),
                               int *passwdenc __attribute__ ((unused)),
                               const char **passwdPtr,
                               const char **refpassPtr)
{
    if (!strcmp(*passwdPtr, *refpassPtr) ||
        pass_ok(*passwdPtr, *refpassPtr))
    {
        hookStop();
        return 1;
    }

    hookStop();
    return 0;
}
