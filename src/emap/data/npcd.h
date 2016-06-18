// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_NPCD
#define EVOL_MAP_NPCD

struct NpcdExt;

struct NpcdExt *npcd_get(TBL_NPC *nd);
struct NpcdExt *npcd_create(void);
void npcd_copy(TBL_NPC *snd,
               TBL_NPC *nd);

#endif  // EVOL_MAP_NPCD
