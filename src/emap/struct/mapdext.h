// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_MAPDEXT
#define EVOL_MAP_MAPDEXT

#include "common/db.h"

struct MapdExt
{
    unsigned int mask;
    VECTOR_DECL(int) npcs;
    struct MapdExtFlag
    {
        unsigned nopve : 1;
    } flag;
    bool invisible;
};

#endif  // EVOL_MAP_MAPDEXT
