// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_STRUTILDATA
#define EVOL_MAP_STRUTILDATA

#include "common/db.h"

struct strutil_data
{
    char *str;
    VECTOR_DECL(char*) parts;
    int len;
};

#endif  // EVOL_MAP_STRUTILDATA
