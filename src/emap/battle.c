// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/nullpo.h"
#include "common/utils.h"

#include "map/itemdb.h"
#include "map/mob.h"
#include "map/pc.h"

#include "plugins/HPMHooking.h"

#include "emap/data/itemd.h"
#include "emap/data/mobd.h"
#include "emap/struct/itemdext.h"
#include "emap/struct/mobdext.h"

// copy from common/utils.c
/**
 * Applies a percentual rate modifier.
 *
 * @param value The base value.
 * @param rate  The rate modifier to apply.
 * @param stdrate The rate modifier's divider (rate == stdrate => 100%).
 * @return The modified value.
 */
int64 apply_percentrate64(int64 value, int rate, int stdrate)
{
    Assert_ret(stdrate > 0);
    Assert_ret(rate >= 0);
    if (rate == stdrate)
        return value;
    if (rate == 0)
        return 0;
    if (INT64_MAX / rate < value)
    {
        // Give up some precision to prevent overflows
        return value / stdrate * rate;
    }
    return value * rate / stdrate;
}

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
        if (weaponIndex < 0)
            return true;

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
    return false;
}

struct Damage ebattle_calc_weapon_attack_post(struct Damage retVal,
                                              struct block_list *src,
                                              struct block_list *target,
                                              uint16 skill_id,
                                              uint16 skill_lv,
                                              int wflag)
{
    if (src == NULL)
        return retVal;

    struct map_session_data *sd = BL_CAST(BL_PC, src);
    if (sd == NULL)
        return retVal;

    // Staffs and Books will use MATK instead of ATK.
    // These regular attacks are still subject to regular DEF/Evade/etc.
    // And base attack bonus might still be gained from STR instead of INT.
    // TODO: Move this to a pre-hook to avoid extra calculation
    if (!skill_id && (sd->weapontype == W_STAFF || sd->weapontype == W_BOOK)) {
        retVal=battle->calc_magic_attack(src, target, skill_id, skill_lv, wflag);
    }

    struct mob_data *md = BL_CAST(BL_MOB, target);
    if (md == NULL)
        return retVal;

    struct MobdExt *data = mobd_get_by_mob(md);
    if (data == NULL)
        return retVal;

    int mod = 0;
    if (skill_id > 0)
    {
        const int idx = skill->get_index(skill_id);
        mod = data->skillAttacks[idx];
    }
    else
    {
        mod = data->weaponAttacks[sd->weapontype1];
    }
    retVal.damage = apply_percentrate64(retVal.damage, mod, 10000);
    retVal.damage2 = apply_percentrate64(retVal.damage2, mod, 10000);
    return retVal;
}

enum damage_lv ebattle_weapon_attack_pre(struct block_list **srcPtr,
                                         struct block_list **targetPtr,
                                         int64 *tickPtr __attribute__ ((unused)),
                                         int *flagPtr __attribute__ ((unused)))
{
    struct block_list *src = *srcPtr;
    struct block_list *target = *targetPtr;

    nullpo_retr(ATK_NONE, src);
    nullpo_retr(ATK_NONE, target);

    struct map_session_data *sd = BL_CAST(BL_PC, src);
    if (sd == NULL)
        return ATK_NONE;

    const int weaponIndex = sd->equip_index[EQI_HAND_L];
    if (weaponIndex < 0)
        return ATK_NONE;

    struct ItemdExt *data = itemd_get(sd->inventory_data[weaponIndex]);
    if (!data)
        return ATK_NONE;

    if (distance_bl(src, target) < data->minRange)
    {   // if range between player and target > minRange, then dont attack
        hookStop();
        return ATK_NONE;
    }
    return ATK_NONE;
}
