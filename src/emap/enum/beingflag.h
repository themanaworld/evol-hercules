// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2019 Evol developers

// This is a ManaPlus enum:
// https://gitlab.com/manaplus/manaplus/blob/master/src/being/beingflag.h

#ifndef EVOL_MAP_ENUM_BEINGFLAG
#define EVOL_MAP_ENUM_BEINGFLAG

typedef enum BeingFlag
{
    BEINGFLAG_SHOP          = 1,
    BEINGFLAG_AWAY          = 2,
    BEINGFLAG_INACTIVE      = 4,
    BEINGFLAG_GENDER_HIDDEN = 32,
    BEINGFLAG_GM            = 64,
    BEINGFLAG_GENDER_MALE   = 128,
    BEINGFLAG_SPECIAL       = 128 + 64
} BeingFlag;

#endif  // EVOL_MAP_ENUM_BEINGFLAG
