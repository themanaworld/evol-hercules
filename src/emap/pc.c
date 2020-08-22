// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/achievement.h"
#include "map/chrif.h"
#include "map/homunculus.h"
#include "map/elemental.h"
#include "map/itemdb.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/quest.h"

#include "plugins/HPMHooking.h"

#include "ecommon/enum/gender.h"

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

int64 epc_readparam_pre(const TBL_PC **sdPtr,
                        int *type)
{
    if (*type == Const_ClientVersion)
    {
        struct SessionExt *data = session_get_bysd((TBL_PC*)(*sdPtr));
        hookStop();
        if (!data)
            return 0;
        return data->clientVersion;
    }
    return 0;
}

/**
 * change sex without the complicated RO stuff
 * @return 1 on success
**/
static int epc_changesex(TBL_PC *sd, unsigned char sex)
{
    // because this is evol we do not any of the bulky sex change stuff
    // since every gender can use every job and every item
    switch (sex) {
        case GENDER_FEMALE:
        case GENDER_MALE:
        case GENDER_HIDDEN:
            sd->status.sex = sex;
            break;
        default:
            return 0;
    }

    // FIXME: there's no way to tell manaplus we changed gender!
    //clif->updatestatus(sd, SP_SEX);

    // FIXME: show gender change to other players:
    //pc->stop_following(sd); // fixpos

    // TODO: add a packet to manaplus that calls dstBeing->setGender() in ea/beingrecv or modify an existing packet and bump the protocol version

    // tell char server we changed sex
    chrif->changesex(sd, 0);

    // XXX: we kick to the char server because we currently can't update in manaplus
    eclif_force_charselect(sd);
    return 1;
}

int epc_setparam_pre(TBL_PC **sd, int *type, int64 *val)
{
    if (*type == SP_SEX)
    {
        int ret = epc_changesex(*sd, *val);
        hookStop();
        return ret;
    }
    return 1;
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
        send_pc_info(&sd->bl, &sd->bl, AREA);
        send_pc_own_flags(&sd->bl);
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
            send_pc_own_flags(&sd->bl);
        }
    }

    return 0;
}

#define equipPos(mask, field, lookf) \
    if (pos & (mask)) \
    { \
        if (id) \
            sd->status.field = id->view_sprite; \
        else \
            sd->status.field = 0; \
        eclif_changelook2(&sd->bl, lookf, sd->status.field, id, n); \
    }

#define equipPos2(mask, lookf) \
    if (pos & (mask)) \
    { \
        if (id) \
            eclif_changelook2(&sd->bl, lookf, id->view_sprite, id, n); \
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

    if (pos & (EQP_HAND_R | EQP_SHADOW_WEAPON))
    {
        if (id != NULL)
        {
            sd->weapontype1 = id->subtype;
            sd->status.look.weapon = id->view_sprite;
        }
        else
        {
            sd->weapontype1 = W_FIST;
            sd->status.look.weapon = 0;
        }
        pc->calcweapontype(sd);
        eclif_changelook2(&sd->bl, LOOK_WEAPON, sd->status.look.weapon, id, n);
    }
    if (pos & (EQP_HAND_L | EQP_SHADOW_SHIELD))
    {
        if (id != NULL)
        {
            if (id->type == IT_WEAPON)
            {
                sd->has_shield = false;
                sd->status.look.shield = 0;
                sd->weapontype2 = id->subtype;
            }
            else if (id->type == IT_ARMOR)
            {
                sd->has_shield = true;
                sd->status.look.shield = id->view_sprite;
                sd->weapontype2 = W_FIST;
            }
        }
        else
        {
            sd->has_shield = false;
            sd->status.look.shield = 0;
            sd->weapontype2 = W_FIST;
        }
        pc->calcweapontype(sd);
        eclif_changelook2(&sd->bl, LOOK_SHIELD, sd->status.look.shield, id, n);
    }

    equipPos(EQP_HEAD_LOW, look.head_bottom, LOOK_HEAD_BOTTOM);
    equipPos(EQP_HEAD_TOP, look.head_top, LOOK_HEAD_TOP);
    equipPos(EQP_HEAD_MID, look.head_mid, LOOK_HEAD_MID);
    equipPos(EQP_GARMENT, look.robe, LOOK_ROBE);
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
        sd->weapontype1 = W_FIST;
        pc->calcweapontype(sd);
        sd->status.look.weapon = 0;
        eclif_changelook2(&sd->bl, LOOK_WEAPON, sd->status.look.weapon, 0, n);
        if (!battle->bc->dancing_weaponswitch_fix)
            status_change_end(&sd->bl, SC_DANCING, INVALID_TIMER);
    }
    if (pos & EQP_HAND_L)
    {
        sd->has_shield = false;
        sd->status.look.shield = 0;
        sd->weapontype2 = W_FIST;
        pc->calcweapontype(sd);
        eclif_changelook2(&sd->bl, LOOK_SHIELD, sd->status.look.shield, 0, n);
    }

    unequipPos(EQP_HEAD_LOW, look.head_bottom, LOOK_HEAD_BOTTOM);
    unequipPos(EQP_HEAD_TOP, look.head_top, LOOK_HEAD_TOP);
    unequipPos(EQP_HEAD_MID, look.head_mid, LOOK_HEAD_MID);
    unequipPos(EQP_GARMENT, look.robe, LOOK_ROBE);
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
        if (pc->job_is_dummy(i))
            continue; //Classes that do not need exp tables.
        int j = pc->class2idx(i);
        if (pc->dbs->class_exp_table[j][CLASS_EXP_TABLE_BASE] == NULL)
            ShowWarning("Class %d does not has a base exp table.\n", i);
        if (pc->dbs->class_exp_table[j][CLASS_EXP_TABLE_JOB] == NULL)
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

        if (n < 0 || n >= sd->status.inventorySize)
            return 0;

        struct ItemdExt *data = itemd_get(sd->inventory_data[n]);
        if (!data)
            return retVal;

        // test for missing basic stats calculation
        if (sd->regen.sregen == NULL)
        {
            return retVal;
        }
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

    if (n < 0 || n >= sd->status.inventorySize)
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

    if (n < 0 || n >= sd->status.inventorySize)
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
                     const struct item *item_data,
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
        if (!data)
            return retVal;

        const int sz = VECTOR_LENGTH(data->allowedCards);

        if (sz == 0) // allow cards if AllowedCards list is empty
            return retVal;

        const int newCardId = sd->status.inventory[idx_card].nameid;
        int cardAmountLimit = 0;

        struct item_group *card_group = NULL;

        for (f = 0; f < sz; f ++)
        {
            struct ItemCardExt *const card = &VECTOR_INDEX(data->allowedCards, f);
            struct item_data *card_itemdata = itemdb->search(card->id);

            if (card_itemdata->group != NULL) {
                card_group = card_itemdata->group;

                for (int c = 0; c < card_group->qty; c++) {
                    if (card_group->nameid[c] == newCardId) {
                        cardAmountLimit = card->amount;
                        break;
                    }
                }

                if (cardAmountLimit) {
                    // we found it in a group, so break
                    break;
                }
            } else if (card->id == newCardId) {
                cardAmountLimit = card->amount;
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
            if (cardId == newCardId) {
                cardsAmount ++;
            } else if (card_group != NULL) {
                // use the same counter for all cards that belong to the group
                for (int c = 0; c < card_group->qty; c++) {
                    if (card_group->nameid[c] == cardId) {
                        cardsAmount++;
                        break;
                    }
                }
            }
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
    if (!sd || n < 0 || n >= sd->status.inventorySize)
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
        *idx_equip >= sd->status.inventorySize ||
        *idx_card < 0 ||
        *idx_card >= sd->status.inventorySize)
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

    // Achievements [Smokexyz/Hercules]
    achievement->validate_adopt(p1_sd, true); // Parent 1
    achievement->validate_adopt(p2_sd, true); // Parent 2
    achievement->validate_adopt(b_sd, false); // Baby

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

/*==========================================
 * Called when player changes job
 * Rewrote to make it tidider [Celest]
 *------------------------------------------*/
int epc_jobchange(struct map_session_data *sd,
                  int class,
                  int upper __attribute__ ((unused)))
{
    int i, fame_flag=0;
    int job, idx = 0;

    nullpo_ret(sd);

    if (class < 0)
        return 1;

    //Normalize job.
    job = pc->jobid2mapid(class);
    if (job == -1)
        return 1;
/*
    switch (upper)
    {
        case 1:
            job |= JOBL_UPPER;
            break;
        case 2:
            job |= JOBL_BABY;
            break;
    }
    //This will automatically adjust bard/dancer classes to the correct gender
    //That is, if you try to jobchange into dancer, it will turn you to bard.
    class = pc->mapid2jobid(job, sd->status.sex);
    if (class == -1)
        return 1;
*/

    if ((uint16)job == sd->job)
        return 1; //Nothing to change.

/*
    if ((job & JOBL_2) && !(sd->job & JOBL_2) && (job & MAPID_UPPERMASK) != MAPID_SUPER_NOVICE)
    {
        // changing from 1st to 2nd job
        sd->change_level_2nd = sd->status.job_level;
        pc_setglobalreg(sd, script->add_variable("jobchange_level"), sd->change_level_2nd);
    }
    else if ((job & JOBL_THIRD) != 0 && (sd->job & JOBL_THIRD) == 0)
    {
        // changing from 2nd to 3rd job
        sd->change_level_3rd = sd->status.job_level;
        pc_setglobalreg(sd, script->add_variable("jobchange_level_3rd"), sd->change_level_3rd);
    }
*/

    if(sd->cloneskill_id)
    {
        idx = skill->get_index(sd->cloneskill_id);
        if (sd->status.skill[idx].flag == SKILL_FLAG_PLAGIARIZED)
        {
            sd->status.skill[idx].id = 0;
            sd->status.skill[idx].lv = 0;
            sd->status.skill[idx].flag = 0;
            clif->deleteskill(sd, sd->cloneskill_id);
        }
        sd->cloneskill_id = 0;
        pc_setglobalreg(sd, script->add_variable("CLONE_SKILL"), 0);
        pc_setglobalreg(sd, script->add_variable("CLONE_SKILL_LV"), 0);
    }

    if(sd->reproduceskill_id)
    {
        idx = skill->get_index(sd->reproduceskill_id);
        if (sd->status.skill[idx].flag == SKILL_FLAG_PLAGIARIZED)
        {
            sd->status.skill[idx].id = 0;
            sd->status.skill[idx].lv = 0;
            sd->status.skill[idx].flag = 0;
            clif->deleteskill(sd, sd->reproduceskill_id);
        }
        sd->reproduceskill_id = 0;
        pc_setglobalreg(sd, script->add_variable("REPRODUCE_SKILL"),0);
        pc_setglobalreg(sd, script->add_variable("REPRODUCE_SKILL_LV"),0);
    }

/*
    if ((job & MAPID_UPPERMASK) != (sd->job & MAPID_UPPERMASK))
    { //Things to remove when changing class tree.
        const int class_idx = pc->class2idx(sd->status.class_);
        short id;
        for (i = 0; i < MAX_SKILL_TREE && (id = pc->skill_tree[class_][i].id) > 0; i++)
        {
            //Remove status specific to your current tree skills.
            enum sc_type sc = status->skill2sc(id);
            if (sc > SC_COMMON_MAX && sd->sc.data[sc])
                status_change_end(&sd->bl, sc, INVALID_TIMER);
        }
    }

    if ((sd->job & MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR && (job & MAPID_UPPERMASK) != MAPID_STAR_GLADIATOR)
    {
        // going off star glad lineage, reset feel to not store no-longer-used vars in the database
        pc->resetfeel(sd);
    }
*/

    sd->status.class = class;
    {
        int fame_list_type = pc->famelist_type(sd->job);
        if (fame_list_type != RANKTYPE_UNKNOWN)
            fame_flag = pc->fame_rank(sd->status.char_id, fame_list_type);
    }
    sd->job = (uint16)job;

//    sd->status.job_level = 1;
//    sd->status.job_exp = 0;

    if (sd->status.base_level > pc->maxbaselv(sd))
    {
        sd->status.base_level = pc->maxbaselv(sd);
        sd->status.base_exp=0;
        pc->resetstate(sd);
        clif->updatestatus(sd, SP_STATUSPOINT);
        clif->updatestatus(sd, SP_BASELEVEL);
        clif->updatestatus(sd, SP_BASEEXP);
        clif->updatestatus(sd, SP_NEXTBASEEXP);
    }

    clif->updatestatus(sd, SP_JOBLEVEL);
    clif->updatestatus(sd, SP_JOBEXP);
    clif->updatestatus(sd, SP_NEXTJOBEXP);

    for (i = 0; i < EQI_MAX; i ++)
    {
        if (sd->equip_index[i] >= 0)
        {
            if (!pc->isequip(sd, sd->equip_index[i]))
                pc->unequipitem(sd, sd->equip_index[i], PCUNEQUIPITEM_FORCE); // unequip invalid item for class
        }
    }

    //Change look, if disguised, you need to undisguise
    //to correctly calculate new job sprite without
    if (sd->disguise != -1)
        pc->disguise(sd, -1);

    // Fix atcommand @jobchange when the player changing from 3rd job having alternate body style into non-3rd job, crashing the client
//    if (pc->has_second_costume(sd) == false) {
//        sd->status.body = 0;
//        sd->vd.body_style = 0;
//        clif->changelook(&sd->bl, LOOK_BODY2, sd->vd.body_style);
//    }

    status->set_viewdata(&sd->bl, class);
    send_changelook2(sd, &sd->bl, sd->bl.id, LOOK_BASE, sd->vd.class, 0, NULL, 0, AREA);
//    clif->changelook(&sd->bl, LOOK_BASE, sd->vd.class); // move sprite update to prevent client crashes with incompatible equipment [Valaris]
    if (sd->vd.cloth_color)
        clif->changelook(&sd->bl, LOOK_CLOTHES_COLOR, sd->vd.cloth_color);
    if (sd->vd.body_style)
        clif->changelook(&sd->bl, LOOK_BODY2, sd->vd.body_style);

    //Update skill tree.
    pc->calc_skilltree(sd);
    clif->skillinfoblock(sd);

    if (sd->ed)
        elemental->delete(sd->ed, 0);
    if (sd->state.vending)
        vending->close(sd);

    map->foreachinmap(pc->jobchange_killclone, sd->bl.m, BL_MOB, sd->bl.id);

    //Remove peco/cart/falcon
    i = sd->sc.option;
    if (i & OPTION_RIDING && (!pc->checkskill(sd, KN_RIDING) || (sd->job & MAPID_THIRDMASK) == MAPID_RUNE_KNIGHT))
        i &= ~ OPTION_RIDING;
    if (i & OPTION_FALCON && !pc->checkskill(sd, HT_FALCON))
        i &= ~ OPTION_FALCON;
    if (i & OPTION_DRAGON && !pc->checkskill(sd,RK_DRAGONTRAINING))
        i &= ~ OPTION_DRAGON;
    if (i & OPTION_WUGRIDER && !pc->checkskill(sd,RA_WUGMASTERY))
        i &= ~ OPTION_WUGRIDER;
    if (i & OPTION_WUG && !pc->checkskill(sd,RA_WUGMASTERY))
        i &= ~ OPTION_WUG;
    if (i & OPTION_MADOGEAR) //You do not need a skill for this.
        i &= ~ OPTION_MADOGEAR;
//#ifndef NEW_CARTS
//    if (i & OPTION_CART && !pc->checkskill(sd, MC_PUSHCART))
//        i &= ~ OPTION_CART;
//#else
    if (sd->sc.data[SC_PUSH_CART] && !pc->checkskill(sd, MC_PUSHCART))
        pc->setcart(sd, 0);
//#endif
    if (i != sd->sc.option)
        pc->setoption(sd, i);

    if (homun_alive(sd->hd) && !pc->checkskill(sd, AM_CALLHOMUN))
        homun->vaporize(sd, HOM_ST_REST, true);

    if ((sd->sc.data[SC_SPRITEMABLE] && pc->checkskill(sd, SU_SPRITEMABLE)))
        status_change_end(&sd->bl, SC_SPRITEMABLE, INVALID_TIMER);

    if (sd->status.manner < 0)
        clif->changestatus(sd, SP_MANNER, sd->status.manner);

    status_calc_pc(sd, SCO_FORCE);
    pc->checkallowskill(sd);
    pc->equiplookall(sd);

    //if you were previously famous, not anymore.
    if (fame_flag != 0)
    {
        chrif->save(sd, 0);
        chrif->buildfamelist();
    }
    else if (sd->status.fame > 0)
    {
        //It may be that now they are famous?
        switch (sd->job & MAPID_UPPERMASK)
        {
            case MAPID_BLACKSMITH:
            case MAPID_ALCHEMIST:
            case MAPID_TAEKWON:
                chrif->save(sd, 0);
                chrif->buildfamelist();
            break;
        }
    }

    quest->questinfo_refresh(sd);

    achievement->validate_jobchange(sd); // Achievements [Smokexyz/Hercules]

    return 0;
}

// copy from pc_calc_skilltree_clear, disabled NV_TRICKDEAD.
void epc_calc_skilltree_clear_pre(struct map_session_data **sdPtr)
{
    struct map_session_data *sd = *sdPtr;
    nullpo_retv(sd);

    for (int i = 0; i < MAX_SKILL_DB; i++)
    {
        if (sd->status.skill[i].flag != SKILL_FLAG_PLAGIARIZED && sd->status.skill[i].flag != SKILL_FLAG_PERM_GRANTED) //Don't touch these
            sd->status.skill[i].id = 0; //First clear skills.
        /* permanent skills that must be re-checked */
//        if (sd->status.skill[i].flag == SKILL_FLAG_PERMANENT)
//        {
//            switch (skill->dbs->db[i].nameid)
//            {
//                case NV_TRICKDEAD:
//                    if ((sd->job & MAPID_UPPERMASK) != MAPID_NOVICE)
//                    {
//                        sd->status.skill[i].id = 0;
//                        sd->status.skill[i].lv = 0;
//                        sd->status.skill[i].flag = 0;
//                    }
//                    break;
//            }
//        }
    }
    hookStop();
}

// disable job based bonuses
void epc_calc_skilltree_bonus_pre(struct map_session_data **sdPtr __attribute__ ((unused)),
                                  int *classidxPtr __attribute__ ((unused)))
{
    hookStop();
}

void epc_checkbaselevelup_sc_pre(struct map_session_data **sdPtr __attribute__ ((unused)))
{
    hookStop();
}


// disable job based resets
bool epc_resetskill_job_pre(struct map_session_data** sdPtr __attribute__ ((unused)),
                            int *indexPtr __attribute__ ((unused)))
{
    hookStop();
    return false;
}

bool epc_isDeathPenaltyJob_pre(uint16 *jobPtr __attribute__ ((unused)))
{
    hookStop();
    return true;
}

bool epc_read_skill_job_skip_pre(short *skill_idPtr __attribute__ ((unused)),
                                 int *job_idPtr __attribute__ ((unused)))
{
    hookStop();
    return false;
}
