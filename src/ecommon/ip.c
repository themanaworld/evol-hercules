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

#include "ecommon/ip.h"

bool checkAllowedIp(const char* const mask, const char* const ip)
{
    char buf[1000];

    strcpy(buf, ",");
    strcat(buf, ip);
    strcat(buf, ",");

    return strstr(mask, buf) != NULL;
}
