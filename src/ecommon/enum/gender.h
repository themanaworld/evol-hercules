// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2020 Evol developers

#ifndef EVOL_ENUM_GENDER
#define EVOL_ENUM_GENDER

#include "common/mmo.h"

/**
 *  identical to SEX_ but also includes nonbinary
 */
typedef enum Gender
{
    GENDER_FEMALE = SEX_FEMALE,
    GENDER_MALE = SEX_MALE,
    __UNUSED_GENDER_SERVER = SEX_SERVER,
    GENDER_NONBINARY,
} Gender;

#endif  // EVOL_ENUM_GENDER
