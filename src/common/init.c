// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../common/HPMi.h"
#include "../../../common/conf.h"
#include "../../../common/malloc.h"
#include "../../../common/mapindex.h"
#include "../../../common/mmo.h"
#include "../../../common/socket.h"
#include "../../../common/strlib.h"
#include "../../../common/sysinfo.h"
#include "../../../common/timer.h"

void interfaces_init_common(void)
{
    iMalloc = GET_SYMBOL("iMalloc");
    strlib = GET_SYMBOL("strlib");
    session = GET_SYMBOL("session");
    sockt = GET_SYMBOL("sockt");
    sv = GET_SYMBOL("sv");
    StrBuf = GET_SYMBOL("StrBuf");
    SQL = GET_SYMBOL("SQL");
    timer = GET_SYMBOL("timer");
    libconfig = GET_SYMBOL("libconfig");
    sysinfo = GET_SYMBOL("sysinfo");
    DB = GET_SYMBOL("DB");
}

void setDefaultMap(void)
{
    mapindex->default_map = "000-1";
    mapindex->default_x = 80;
    mapindex->default_y = 109;
}
