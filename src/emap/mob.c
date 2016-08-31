// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/battle.h"
#include "map/itemdb.h"
#include "map/mob.h"
#include "map/script.h"

#include "plugins/HPMHooking.h"

#include "emap/mob.h"

#include "emap/data/mobd.h"
#include "emap/struct/mobdext.h"

int emob_deleteslave_sub_pre(struct block_list **blPtr,
                             va_list ap)
{
    struct block_list *bl = *blPtr;
    if (!bl)
    {
        hookStop();
        return 0;
    }
    TBL_MOB *md = (TBL_MOB *)bl;
    if (!md)
    {
        hookStop();
        return 0;
    }

    const int id = va_arg(ap, int);
    if (md->master_id > 0 && md->master_id == id)
    {
        if (md->db->status.mode & MD_SURVIVE_WITHOUT_MASTER)
        {
            md->master_id = 0;
            md->master_dist = 0;
        }
        else
        {
            status_kill(bl);
        }
    }

    hookStop();
    return 0;
}

static void emob_load_weaponattacks(const char *type,
                                    int val,
                                    struct MobdExt *data,
                                    struct mob_db *entry)
{
    int key = -1;
    if (strcmp(type, "NoWeapon") == 0)
    {
        key = W_FIST;
    }
    else if (strcmp(type, "Daggers") == 0)
    {
        key = W_DAGGER;
    }
    else if (strcmp(type, "1HSwords") == 0)
    {
        key = W_1HSWORD;
    }
    else if (strcmp(type, "2HSwords") == 0)
    {
        key = W_2HSWORD;
    }
    else if (strcmp(type, "1HSpears") == 0)
    {
        key = W_1HSPEAR;
    }
    else if (strcmp(type, "2HSpears") == 0)
    {
        key = W_2HSPEAR;
    }
    else if (strcmp(type, "1HAxes") == 0)
    {
        key = W_1HAXE;
    }
    else if (strcmp(type, "2HAxes") == 0)
    {
        key = W_2HAXE;
    }
    else if (strcmp(type, "Maces") == 0)
    {
        key = W_MACE;
    }
    else if (strcmp(type, "2HMaces") == 0)
    {
        key = W_2HMACE;
    }
    else if (strcmp(type, "Staves") == 0)
    {
        key = W_STAFF;
    }
    else if (strcmp(type, "Bows") == 0)
    {
        key = W_BOW;
    }
    else if (strcmp(type, "Knuckles") == 0)
    {
        key = W_KNUCKLE;
    }
    else if (strcmp(type, "Instruments") == 0)
    {
        key = W_MUSICAL;
    }
    else if (strcmp(type, "Whips") == 0)
    {
        key = W_WHIP;
    }
    else if (strcmp(type, "Books") == 0)
    {
        key = W_BOOK;
    }
    else if (strcmp(type, "Katars") == 0)
    {
        key = W_KATAR;
    }
    else if (strcmp(type, "Revolvers") == 0)
    {
        key = W_REVOLVER;
    }
    else if (strcmp(type, "Rifles") == 0)
    {
        key = W_RIFLE;
    }
    else if (strcmp(type, "GatlingGuns") == 0)
    {
        key = W_GATLING;
    }
    else if (strcmp(type, "Shotguns") == 0)
    {
        key = W_SHOTGUN;
    }
    else if (strcmp(type, "GrenadeLaunchers") == 0)
    {
        key = W_GRENADE;
    }
    else if (strcmp(type, "FuumaShurikens") == 0)
    {
        key = W_HUUMA;
    }
    else if (strcmp(type, "2HStaves") == 0)
    {
        key = W_2HSTAFF;
    }
    else if (strcmp(type, "DWDaggers") == 0)
    {
        key = W_DOUBLE_DD;
    }
    else if (strcmp(type, "DWSwords") == 0)
    {
        key = W_DOUBLE_SS;
    }
    else if (strcmp(type, "DWAxes") == 0)
    {
        key = W_DOUBLE_AA;
    }
    else if (strcmp(type, "DWDaggerSword") == 0)
    {
        key = W_DOUBLE_DS;
    }
    else if (strcmp(type, "DWDaggerAxe") == 0)
    {
        key = W_DOUBLE_DA;
    }
    else if (strcmp(type, "DWSwordAxe") == 0)
    {
        key = W_DOUBLE_SA;
    }
    else if (strcmp(type, "All") == 0)
    {
        for (int f = 0; f < MAX_WEAPON_TYPE; f ++)
            data->weaponAttacks[f] = val;
    }
    else
    {
        ShowError("Mob %d. Unknown weapon type %s\n", entry->mob_id, type);
        return;
    }
    data->weaponAttacks[key] = val;
}

static void emob_load_skillattacks(const char *type,
                                   int val,
                                   struct MobdExt *data,
                                   struct mob_db *entry)
{
    int skill_id = 0;
    if (script->get_constant(type, &skill_id))
    {
        const int idx = skill->get_index(skill_id);
        data->skillAttacks[idx] = val;
    }
    else
    {
        ShowError("Mob %d. Unknown skill name %s\n", entry->mob_id, type);
    }
}

void emob_read_db_additional_fields_pre(struct mob_db **entryPtr,
                                        struct config_setting_t **itPtr,
                                        int *nPtr __attribute__ ((unused)),
                                        const char **sourcePtr __attribute__ ((unused)))
{
    int i32 = 0;

    struct MobdExt *data = mobd_get(*entryPtr);
    if (!data)
    {
        hookStop();
        return;
    }

    if (mob->lookup_const(*itPtr, "WalkMask", &i32))
        data->walkMask = i32;

    struct config_setting_t *tt = libconfig->setting_get_member(*itPtr, "WeaponAttacks");

    if (tt && config_setting_is_group(tt))
    {
        int j = 0;
        struct config_setting_t *wpt = NULL;
        while ((wpt = libconfig->setting_get_elem(tt, j++)) != NULL)
        {
            emob_load_weaponattacks(config_setting_name(wpt), libconfig->setting_get_int(wpt), data, *entryPtr);
        }
    }

    tt = libconfig->setting_get_member(*itPtr, "SkillAttacks");

    if (tt && config_setting_is_group(tt))
    {
        int j = 0;
        struct config_setting_t *wpt = NULL;
        while ((wpt = libconfig->setting_get_elem(tt, j++)) != NULL)
        {
            emob_load_skillattacks(config_setting_name(wpt), libconfig->setting_get_int(wpt), data, *entryPtr);
        }
    }
}

uint32 emob_read_db_mode_sub_post(uint32 retVal,
                                  struct mob_db *entry  __attribute__ ((unused)),
                                  struct config_setting_t *t)
{
    struct config_setting_t *t2;

    if ((t2 = libconfig->setting_get_member(t, "SurviveWithoutMaster")))
        retVal |= libconfig->setting_get_bool(t2) ? MD_SURVIVE_WITHOUT_MASTER : 0;

    return retVal;
}
