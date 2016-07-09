// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_SKILLD
#define EVOL_MAP_SKILLD

struct SkilldExt;

void skilld_init(void);

struct SkilldExt *skilld_get(const int idx);

struct SkilldExt *skilld_get_id(const int skill_id);

int skilld_get_misceffect(const int skill_id,
                          const int effect_idx);

#endif  // EVOL_MAP_SKILLD
