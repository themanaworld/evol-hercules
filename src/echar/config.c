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

#include "echar/config.h"

int min_char_class = 0;
int max_char_class = 0;
int min_look = 0;
int max_look = 0;

void config_min_char_class(const char *key __attribute__ ((unused)),
                           const char *val)
{
    min_char_class = atoi(val);
}

void config_max_char_class(const char *key __attribute__ ((unused)),
                           const char *val)
{
    max_char_class = atoi(val);
}

void config_min_look(const char *key __attribute__ ((unused)),
                     const char *val)
{
    min_look = atoi(val);
}

void config_max_look(const char *key __attribute__ ((unused)),
                     const char *val)
{
    max_look = atoi(val);
}

void config_final(void)
{
}
