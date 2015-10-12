// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_MOBD
#define EVOL_MAP_MOBD

struct MobdExt;
struct mob_db;

struct MobdExt *mobd_get(struct mob_db *md);
struct MobdExt *mobd_get_by_mob(TBL_MOB *md);
struct MobdExt *mobd_create(void);

#endif  // EVOL_MAP_MOBD
