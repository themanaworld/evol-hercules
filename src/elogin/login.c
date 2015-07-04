// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "login/login.h"

#include "elogin/md5calc.h"

bool elogin_check_password(const char* md5key __attribute__ ((unused)),
                           int *passwdenc __attribute__ ((unused)),
                           const char* passwd,
                           const char* refpass)
{
    if (!strcmp(passwd, refpass) || pass_ok(passwd, refpass))
    {
        hookStop();
        return 1;
    }

    hookStop();
    return 0;
}
