// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_MAPD
#define EVOL_MAP_MAPD

struct MapdExt;

struct MapdExt *mapd_get(int fd);
struct MapdExt *mapd_create(void);

#endif  // EVOL_MAP_MAPD
