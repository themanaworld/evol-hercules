// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"

#include "map/itemdb.h"
#include "map/pc.h"

#include "emap/data/itemd.h"
#include "emap/struct/itemdext.h"

bool ebattle_check_arrows_post(bool retVal,
                               struct map_session_data *sd)
{
    if (retVal == true)
    {
        if (!sd)
            return retVal;
        const int ammoIndex = sd->equip_index[EQI_AMMO];
        if (ammoIndex < 0)
            return true;

        const int ammoId = sd->inventory_data[ammoIndex]->nameid;
        if (ammoId <= 0)
            return true;

        const int weaponIndex = sd->equip_index[EQI_HAND_L];

        struct ItemdExt *data = itemd_get(sd->inventory_data[weaponIndex]);
        if (!data)
            return true;

        const int sz = VECTOR_LENGTH(data->allowedAmmo);

        if (sz == 0) // allow any ammo if AllowedAmmo list is empty
            return true;

        for (int f = 0; f < sz; f ++)
        {
            const int id = VECTOR_INDEX(data->allowedAmmo, f);
            if (ammoId == id)
                return true;
        }
        clif->arrow_fail(sd, 0);
        return false;
    }
    return true;
}
