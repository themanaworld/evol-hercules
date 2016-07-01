// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include "map/script.h"

#include "emap/skill_const.h"

void eskill_addskill_conststants(void)
{
    script->constdb_comment("Evol skills");
    script->set_constant("EVOL_MASSPROVOKE", EVOL_MASSPROVOKE, false, false);
    script->constdb_comment(NULL);
}
