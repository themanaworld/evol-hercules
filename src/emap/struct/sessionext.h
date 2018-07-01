// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_SESSIONEXT
#define EVOL_MAP_SESSIONEXT

struct SessionExt
{
    time_t onlinelistlasttime;
    int clientVersion;
    int language;
    int teamId;
    uint16 mount;
    uint8 state;
    int jump_iterate_id;
};

#endif  // EVOL_MAP_SESSIONEXT
