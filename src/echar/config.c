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

#include "echar/config.h"

char *inter_server_ip = NULL;
int min_char_class = 0;
int max_char_class = 0;
int min_look = 0;
int max_look = 0;

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

void config_min_char_class(const char *val)
{
    min_char_class = atoi(val);
}

void config_max_char_class(const char *val)
{
    max_char_class = atoi(val);
}

void config_min_look(const char *val)
{
    min_look = atoi(val);
}

void config_max_look(const char *val)
{
    max_look = atoi(val);
}

void config_final(void)
{
    if (inter_server_ip)
        aFree(inter_server_ip);
}
