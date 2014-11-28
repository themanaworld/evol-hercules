// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_NPCD
#define EVOL_MAP_NPCD

struct NpcdExt;

struct NpcdExt *npcd_get(int fd);
struct NpcdExt *npcd_create(void);

#endif  // EVOL_MAP_NPCD
