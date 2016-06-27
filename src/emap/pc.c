// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/itemdb.h"
#include "map/npc.h"
#include "map/pc.h"

#include "plugins/HPMHooking.h"

#include "emap/clif.h"
#include "emap/pc.h"
#include "emap/send.h"
#include "emap/script.h"
#include "emap/data/itemd.h"
#include "emap/data/mapd.h"
#include "emap/data/session.h"
#include "emap/struct/itemdext.h"
#include "emap/struct/mapdext.h"
#include "emap/struct/sessionext.h"

int langScriptId;
int mountScriptId;

int epc_readparam_pre(TBL_PC **sdPtr,
                      int *type)
{
    if (*type == Const_ClientVersion)
    {
        struct SessionExt *data = session_get_bysd(*sdPtr);
        hookStop();
        if (!data)
            return 0;
        return data->clientVersion;
    }
    return 0;
}

int epc_setregistry_pre(TBL_PC **sdPtr,
                        int64 *reg,
                        int *val)
{
    TBL_PC *sd = *sdPtr;

    if (*reg == langScriptId)
    {
        struct SessionExt *data = session_get_bysd(sd);
        if (!data)
            return 0;
        data->language = *val;
    }
    else if (*reg == mountScriptId)
    {
        struct SessionExt *data = session_get_bysd(sd);
        if (!data)
            return 0;
        if (data->mount != *val)
        {
            data->mount = *val;
            send_pc_info(&sd->bl, &sd->bl, SELF);
        }
    }

    return 0;
}

#define equipPos(mask, field, lookf) \
    if (pos & (mask)) \
    { \
        if (id) \
            sd->status.field = id->look; \
        else \
            sd->status.field = 0; \
        eclif_changelook2(&sd->bl, lookf, sd->status.field, id, n); \
    }

#define equipPos2(mask, lookf) \
    if (pos & (mask)) \
    { \
        if (id) \
            eclif_changelook2(&sd->bl, lookf, id->look, id, n); \
        else \
            eclif_changelook2(&sd->bl, lookf, 0, id, n); \
    }

void epc_equipitem_pos_pre(TBL_PC **sdPtr,
                           struct item_data **idPtr,
                           int *nPtr,
                           int *posPtr)
{
    const int n = *nPtr;
    int pos = *posPtr;
    TBL_PC *sd = *sdPtr;
    struct item_data *id = *idPtr;

    hookStop();

    if (!id || !sd)
        return;

    if (pos & (EQP_HAND_R|EQP_SHADOW_WEAPON))
    {
        if(id)
        {
            sd->weapontype1 = id->look;
        }
        else
        {
            sd->weapontype1 = 0;
        }
        pc->calcweapontype(sd);
        eclif_changelook2(&sd->bl, LOOK_WEAPON, sd->status.weapon, id, n);
    }
    if (pos & (EQP_HAND_L|EQP_SHADOW_SHIELD))
    {
        if (id)
        {
            if(id->type == IT_WEAPON)
            {
                sd->status.shield = 0;
                sd->weapontype2 = id->look;
            }
            else if (id->type == IT_ARMOR)
            {
                sd->status.shield = id->look;
                sd->weapontype2 = 0;
            }
        }
        else
        {
            sd->status.shield = sd->weapontype2 = 0;
        }
        pc->calcweapontype(sd);
        eclif_changelook2(&sd->bl, LOOK_SHIELD, sd->status.shield, id, n);
    }

    equipPos(EQP_HEAD_LOW, head_bottom, LOOK_HEAD_BOTTOM);
    equipPos(EQP_HEAD_TOP, head_top, LOOK_HEAD_TOP);
    equipPos(EQP_HEAD_MID, head_mid, LOOK_HEAD_MID);
    equipPos(EQP_GARMENT, robe, LOOK_ROBE);
    equipPos2(EQP_SHOES, LOOK_SHOES);
    equipPos2(EQP_COSTUME_HEAD_TOP, 13);
    equipPos2(EQP_COSTUME_HEAD_MID, 14);
    equipPos2(EQP_COSTUME_HEAD_LOW, 15);
    equipPos2(EQP_COSTUME_GARMENT, 16);
    equipPos2(EQP_ARMOR, 17);
    equipPos2(EQP_ACC_R, 18);
    equipPos2(EQP_ACC_L, 19);

    //skipping SHADOW slots
}

#undef equipPos
#undef equipPos2

#define unequipPos(mask, field, lookf) \
    if (pos & (mask)) \
    { \
        sd->status.field = 0; \
        eclif_changelook2(&sd->bl, lookf, sd->status.field, 0, n); \
    }

#define unequipPos2(mask, lookf) \
    if (pos & (mask)) \
        eclif_changelook2(&sd->bl, lookf, 0, 0, n);

void epc_unequipitem_pos_pre(TBL_PC **sdPtr,
                             int *nPtr,
                             int *posPtr)
{
    TBL_PC *sd = *sdPtr;
    if (!sd)
        return;

    hookStop();

    const int n = *nPtr;
    int pos = *posPtr;

    if (pos & EQP_HAND_R)
    {
        sd->weapontype1 = 0;
        sd->status.weapon = sd->weapontype2;
        pc->calcweapontype(sd);
        eclif_changelook2(&sd->bl, LOOK_WEAPON, sd->status.weapon, 0, n);
        if (!battle->bc->dancing_weaponswitch_fix)
            status_change_end(&sd->bl, SC_DANCING, INVALID_TIMER);
    }
    if (pos & EQP_HAND_L)
    {
        sd->status.shield = sd->weapontype2 = 0;
        pc->calcweapontype(sd);
        eclif_changelook2(&sd->bl, LOOK_SHIELD, sd->status.shield, 0, n);
    }

    unequipPos(EQP_HEAD_LOW, head_bottom, LOOK_HEAD_BOTTOM);
    unequipPos(EQP_HEAD_TOP, head_top, LOOK_HEAD_TOP);
    unequipPos(EQP_HEAD_MID, head_mid, LOOK_HEAD_MID);
    unequipPos(EQP_GARMENT, robe, LOOK_ROBE);
    unequipPos2(EQP_SHOES, LOOK_SHOES);
    unequipPos2(EQP_COSTUME_HEAD_TOP, 13);
    unequipPos2(EQP_COSTUME_HEAD_MID, 14);
    unequipPos2(EQP_COSTUME_HEAD_LOW, 15);
    unequipPos2(EQP_COSTUME_GARMENT, 16);
    unequipPos2(EQP_ARMOR, 17);
    unequipPos2(EQP_ACC_R, 18);
    unequipPos2(EQP_ACC_L, 19);
    //skipping SHADOW slots
}

#undef unequipPos
#undef unequipPos2

bool epc_can_attack_pre(TBL_PC **sdPtr,
                        int *target_id)
{
    TBL_PC *sd = *sdPtr;
    if (!sd)
        return false;

    struct MapdExt *data = mapd_get(sd->bl.m);
    if (!data)
        return true;
    if (data->flag.nopve)
    {
        if (map->id2md(*target_id))
        {
            hookStop();
            return false;
        }
    }
    return true;
}


void epc_validate_levels_pre(void)
{
    int i;
    for (i = 0; i < 7; i++) {
        if (!pc->db_checkid(i)) continue;
        if (i == JOB_WEDDING || i == JOB_XMAS || i == JOB_SUMMER)
            continue; //Classes that do not need exp tables.
        int j = pc->class2idx(i);
        if (!pc->max_level[j][0])
            ShowWarning("Class %d does not has a base exp table.\n", i);
        if (!pc->max_level[j][1])
            ShowWarning("Class %d does not has a job exp table.\n", i);
    }
    hookStop();
}

int epc_isequip_post(int retVal,
                     struct map_session_data *sd,
                     int n)
{
    if (retVal)
    {
        if (!sd)
            return 0;

        if (n < 0 || n >= MAX_INVENTORY)
            return 0;

        struct ItemdExt *data = itemd_get(sd->inventory_data[n]);
        if (!data)
            return retVal;

        if (sd->battle_status.str < data->requiredStr ||
            sd->battle_status.agi < data->requiredAgi ||
            sd->battle_status.vit < data->requiredVit ||
            sd->battle_status.int_ < data->requiredInt ||
            sd->battle_status.dex < data->requiredDex ||
            sd->battle_status.luk < data->requiredLuk ||
            sd->battle_status.max_hp < data->requiredMaxHp ||
            sd->battle_status.max_sp < data->requiredMaxSp ||
            sd->battle_status.batk < data->requiredAtk ||
            sd->battle_status.matk_min < data->requiredMAtkMin ||
            sd->battle_status.matk_max < data->requiredMAtkMax ||
            sd->battle_status.def < data->requiredDef ||
            sd->battle_status.mdef < data->requiredMDef ||
            (data->requiredSkill && !pc->checkskill(sd, data->requiredSkill))
        )
        {
            return 0;
        }
    }
    return retVal;
}

int epc_useitem_post(int retVal,
                     struct map_session_data *sd,
                     int n)
{
    if (!sd)
        return retVal;

    if (n < 0 || n >= MAX_INVENTORY)
        return retVal;

    struct ItemdExt *data = itemd_get(sd->inventory_data[n]);
    if (!data)
        return retVal;

    const int effect = retVal ? data->useEffect : data->useFailEffect;
    if (effect != -1)
        clif->specialeffect(&sd->bl, effect, AREA);
    return retVal;
}

static void equippost_effect(struct map_session_data *const sd,
                             const int n,
                             const bool retVal,
                             const bool equip)
{
    if (!sd)
        return;

    if (n < 0 || n >= MAX_INVENTORY)
        return;

    struct ItemdExt *data = itemd_get(sd->inventory_data[n]);
    if (!data)
        return;

    int effect;
    if (equip)
        effect = retVal ? data->useEffect : data->useFailEffect;
    else
        effect = retVal ? data->unequipEffect : data->unequipFailEffect;

    if (effect != -1)
        clif->specialeffect(&sd->bl, effect, AREA);
    return;
}

int epc_equipitem_post(int retVal,
                       struct map_session_data *sd,
                       int n,
                       int data __attribute__ ((unused)))
{
    equippost_effect(sd, n, retVal, true);
    return retVal;
}

int epc_unequipitem_post(int retVal,
                         struct map_session_data *sd,
                         int n,
                         int data __attribute__ ((unused)))
{
    equippost_effect(sd, n, retVal, false);
    return retVal;
}

int epc_check_job_name_pre(const char **namePtr)
{
    int val = -1;
    const char *name = *namePtr;
    if (script->get_constant(name, &val))
    {
        hookStop();
        return val;
    }
    hookStop();
    return -1;
}

int epc_setnewpc_post(int retVal,
                      struct map_session_data *sd,
                      int account_id __attribute__ ((unused)),
                      int char_id __attribute__ ((unused)),
                      int login_id1 __attribute__ ((unused)),
                      unsigned int client_tick  __attribute__ ((unused)),
                      int sex __attribute__ ((unused)),
                      int fd __attribute__ ((unused)))
{
    if (sd)
    {
        sd->battle_status.speed = 150;
        sd->base_status.speed = 150;
    }
    return retVal;
}

int epc_additem_post(int retVal,
                     struct map_session_data *sd,
                     struct item *item_data,
                     int amount __attribute__ ((unused)),
                     e_log_pick_type log_type __attribute__ ((unused)))
{
    if (!retVal)
    {
        struct ItemdExt *data = itemd_get_by_item(item_data);
        if (data && data->charmItem)
            status_calc_pc(sd, SCO_NONE);
    }
    return retVal;
}

static bool calcPc = false;

int epc_delitem_pre(struct map_session_data **sdPtr,
                    int *nPtr,
                    int *amountPtr,
                    int *typePtr __attribute__ ((unused)),
                    short *reasonPtr __attribute__ ((unused)),
                    e_log_pick_type *log_type __attribute__ ((unused)))
{
    struct map_session_data *sd = *sdPtr;
    if (!sd)
        return 1;
    const int n = *nPtr;
    const int amount = *amountPtr;

    if (sd->status.inventory[n].nameid == 0 ||
        amount <= 0 ||
        sd->status.inventory[n].amount < amount ||
        sd->inventory_data[n] == NULL)
    {
        return 1;
    }

    struct ItemdExt *data = itemd_get(sd->inventory_data[n]);
    if (data && data->charmItem)
        calcPc = true;

    return 0;
}

int epc_delitem_post(int retVal,
                     struct map_session_data *sd,
                     int n __attribute__ ((unused)),
                     int amount __attribute__ ((unused)),
                     int type __attribute__ ((unused)),
                     short reason __attribute__ ((unused)),
                     e_log_pick_type log_type __attribute__ ((unused)))
{
    if (!retVal && calcPc && sd)
        status_calc_pc(sd, SCO_NONE);
    calcPc = false;
    return retVal;
}

bool epc_can_insert_card_into_post(bool retVal,
                                   struct map_session_data* sd,
                                   int idx_card,
                                   int idx_equip)
{
    int f;
    if (retVal)
    {
        if (!sd)
            return retVal;
        struct ItemdExt *data = itemd_get(sd->inventory_data[idx_equip]);
        if (!data || !data->allowedCards[0].id) // allow cards if AllowedCards list is empty
            return retVal;

        const int newCardId = sd->status.inventory[idx_card].nameid;
        int cardAmountLimit = 0;

        for (f = 0; f < 100 && data->allowedCards[f].id; f ++)
        {
            if (data->allowedCards[f].id == newCardId)
            {
                cardAmountLimit = data->allowedCards[f].amount;
                break;
            }
        }
        if (!cardAmountLimit)
            return false;

        int cardsAmount = 0;
        const int slots = sd->inventory_data[idx_equip]->slot;
        for (f = 0; f < slots; f ++)
        {
            const int cardId = sd->status.inventory[idx_equip].card[f];
            if (cardId == newCardId)
                cardsAmount ++;
        }
        return cardAmountLimit > cardsAmount;
    }
    return retVal;
}

// temporary inv index and item id
static int tempN = 0;
static int tempId = 0;
static int tempAmount = 0;

int epc_dropitem_pre(struct map_session_data **sdPtr,
                     int *nPtr,
                     int *amountPtr __attribute__ ((unused)))
{
    struct map_session_data *sd = *sdPtr;
    const int n = *nPtr;
    if (!sd || n < 0 || n >= MAX_INVENTORY)
    {
        tempN = 0;
        tempId = 0;
        return 0;
    }

    tempN = n;
    tempId = sd->status.inventory[n].nameid;
    return 1;
}

int epc_dropitem_post(int retVal,
                      struct map_session_data *sd,
                      int n,
                      int amount)
{
    if (retVal && n == tempN && tempId)
    {
        struct item_data *item = itemdb->search(tempId);
        if (!item)
            return retVal;
        struct ItemdExt *data = itemd_get(item);
        if (!data)
            return retVal;
        script_run_item_amount_script(sd, data->dropScript, tempId, amount);
    }
    return retVal;
}

int epc_takeitem_pre(struct map_session_data **sdPtr  __attribute__ ((unused)),
                     struct flooritem_data **fitemPtr)
{
    struct flooritem_data *fitem = *fitemPtr;
    if (!fitem)
    {
        tempN = 0;
        tempId = 0;
        return 0;
    }

    struct ItemdExt *data = itemd_get_by_item(&fitem->item_data);
    if (!data)
    {
        tempN = -1;
        tempId = fitem->item_data.nameid;
        tempAmount = fitem->item_data.amount;
        return 1;
    }

    if (!data->allowPickup)
    {
        hookStop();
        return 0;
    }

    tempN = -1;
    tempId = fitem->item_data.nameid;
    tempAmount = fitem->item_data.amount;
    return 1;
}

int epc_takeitem_post(int retVal,
                      struct map_session_data *sd,
                      struct flooritem_data *fitem __attribute__ ((unused)))
{
    if (retVal && tempN == -1 && tempId)
    {
        struct item_data *item = itemdb->search(tempId);
        if (!item)
            return retVal;
        struct ItemdExt *data = itemd_get(item);
        if (!data)
            return retVal;
        script_run_item_amount_script(sd, data->takeScript, tempId, tempAmount);
    }
    return retVal;
}

int epc_insert_card_pre(struct map_session_data **sdPtr,
                        int *idx_card,
                        int *idx_equip)
{
    struct map_session_data *sd = *sdPtr;
    if (!sd ||
        *idx_equip < 0 ||
        *idx_equip >= MAX_INVENTORY ||
        *idx_card < 0 ||
        *idx_card >= MAX_INVENTORY)
    {
        tempN = 0;
        tempId = 0;
        tempAmount = 0;
        return 0;
    }
    tempN = *idx_equip;
    tempId = sd->status.inventory[*idx_equip].nameid;
    tempAmount = sd->status.inventory[*idx_card].nameid;
    return 1;
}

int epc_insert_card_post(int retVal,
                         struct map_session_data* sd,
                         int idx_card __attribute__ ((unused)),
                         int idx_equip)
{
    if (retVal && idx_equip == tempN && tempId)
    {
        struct item_data *item = itemdb->search(tempId);
        if (!item)
            return retVal;
        struct ItemdExt *data = itemd_get(item);
        if (!data)
            return retVal;
        script_run_card_script(sd, data->insertScript, tempId, tempAmount);
    }
    return retVal;
}

bool epc_can_Adopt_pre(struct map_session_data **p1_sdPtr,
                       struct map_session_data **p2_sdPtr,
                       struct map_session_data **b_sdPtr)
{
    struct map_session_data *p1_sd = *p1_sdPtr;
    struct map_session_data *p2_sd = *p2_sdPtr;
    struct map_session_data *b_sd = *b_sdPtr;

    hookStop();

    if (!p1_sd || !p2_sd || !b_sd)
        return false;

    if (b_sd->status.father || b_sd->status.mother || b_sd->adopt_invite)
        return false; // already adopted baby / in adopt request

    if (!p1_sd->status.partner_id || !p1_sd->status.party_id || p1_sd->status.party_id != b_sd->status.party_id)
        return false; // You need to be married and in party with baby to adopt

    if (p1_sd->status.partner_id != p2_sd->status.char_id || p2_sd->status.partner_id != p1_sd->status.char_id)
        return false; // Not married, wrong married

    if (p2_sd->status.party_id != p1_sd->status.party_id)
        return false; // Both parents need to be in the same party

    // Parents need to have their ring equipped
//    if (!pc->isequipped(p1_sd, WEDDING_RING_M) && !pc->isequipped(p1_sd, WEDDING_RING_F))
//        return false;

//    if (!pc->isequipped(p2_sd, WEDDING_RING_M) && !pc->isequipped(p2_sd, WEDDING_RING_F))
//        return false;

    // Already adopted a baby
    if (p1_sd->status.child || p2_sd->status.child)
    {
        clif->adopt_reply(p1_sd, 0);
        hookStop();
        return false;
    }

    // Parents need at least lvl 70 to adopt
//    if (p1_sd->status.base_level < 70 || p2_sd->status.base_level < 70)
//    {
//        clif->adopt_reply(p1_sd, 1);
//        hookStop();
//        return false;
//    }

    if (b_sd->status.partner_id)
    {
        clif->adopt_reply(p1_sd, 2);
        hookStop();
        return false;
    }

//    if (!((b_sd->status.class_ >= JOB_NOVICE && b_sd->status.class_ <= JOB_THIEF) || b_sd->status.class_ == JOB_SUPER_NOVICE))
//        return false;

    hookStop();
    return true;
}

bool epc_adoption_pre(struct map_session_data **p1_sdPtr,
                      struct map_session_data **p2_sdPtr,
                      struct map_session_data **b_sdPtr)
{
    struct map_session_data *p1_sd = *p1_sdPtr;
    struct map_session_data *p2_sd = *p2_sdPtr;
    struct map_session_data *b_sd = *b_sdPtr;

    if (!pc->can_Adopt(p1_sd, p2_sd, b_sd))
    {
        hookStop();
        return false;
    }

    p1_sd->status.child = b_sd->status.char_id;
    p2_sd->status.child = b_sd->status.char_id;
    b_sd->status.father = p1_sd->status.char_id;
    b_sd->status.mother = p2_sd->status.char_id;

    // Baby Skills
    pc->skill(b_sd, WE_BABY, 1, SKILL_GRANT_PERMANENT);
    pc->skill(b_sd, WE_CALLPARENT, 1, SKILL_GRANT_PERMANENT);

    // Parents Skills
    pc->skill(p1_sd, WE_CALLBABY, 1, SKILL_GRANT_PERMANENT);
    pc->skill(p2_sd, WE_CALLBABY, 1, SKILL_GRANT_PERMANENT);

    hookStop();
    return true;
}

// copy from pc_process_chat_message
// exception only prevent call gm command if string start with ##
bool epc_process_chat_message_pre(struct map_session_data **sdPtr,
                                  const char **messagePtr)
{
    struct map_session_data *sd = *sdPtr;
    const char *message = *messagePtr;
    if (message && strlen(message) > 2 && message[0] == '#' && message[1] == '#')
    {
        // do nothing
    }
    else if (atcommand->exec(sd->fd, sd, message, true))
    {
        hookStop();
        return false;
    }

    if (!pc->can_talk(sd))
    {
        hookStop();
        return false;
    }

    if (battle->bc->min_chat_delay != 0)
    {
        if (DIFF_TICK(sd->cantalk_tick, timer->gettick()) > 0)
        {
            hookStop();
            return false;
        }
        sd->cantalk_tick = timer->gettick() + battle->bc->min_chat_delay;
    }

    pc->update_idle_time(sd, BCIDLE_CHAT);

    hookStop();
    return true;
}

int epc_dead_post(int retVal,
                  struct map_session_data *sd,
                  struct block_list *src)
{
    if (retVal > 0)
    {
        if (sd)
            send_pc_killed(sd->fd, src);
    }
    return retVal;
}
