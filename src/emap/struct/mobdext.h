// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_MOBDEXT
#define EVOL_MAP_MOBDEXT

struct MobdExt
{
    int walkMask;
    int weaponAttacks[MAX_WEAPON_TYPE];
    int skillAttacks[MAX_SKILL_DB];
    int collisionDx;
    int collisionDy;
    int collisionMask;
};

#endif  // EVOL_MAP_MOBDEXT
