// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_MAPDEXT
#define EVOL_MAP_MAPDEXT

struct MapdExt
{
    unsigned int mask;
    bool invisible;
    struct MapdExtFlag
    {
        unsigned nopve : 1;
    } flag;
};

#endif  // EVOL_MAP_MAPDEXT
