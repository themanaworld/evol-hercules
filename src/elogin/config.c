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

#include "elogin/config.h"

char *update_server = NULL;
char *inter_server_ip = NULL;

void config_update_server(const char *val)
{
    update_server = aStrdup(val);
}

void config_inter_server_ip(const char *val)
{
    char buf[1000];

    if (strlen(val) > 900)
        return;

    strcpy(buf, ",");
    strcat(buf, val);
    strcat(buf, ",");
    inter_server_ip = aStrdup(buf);
}

void config_final(void)
{
    if (update_server)
        aFree(update_server);
    if (inter_server_ip)
        aFree(inter_server_ip);
}
