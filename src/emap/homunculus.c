// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/nullpo.h"

#include "emap/send.h"

int ehomunculus_gainexp_pre(struct homun_data **hdPtr,
                            unsigned int *expPtr)
{
    struct homun_data *hd = *hdPtr;
    nullpo_ret(hd);
    const int exp = *expPtr;

    send_homun_exp(hd, exp);
    return 0;
}
