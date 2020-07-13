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
#include "common/cbasetypes.h"
#include "common/random.h"
#include "common/timer.h"
#include "map/battle.h"
#include "map/chrif.h"
#include "map/elemental.h"
#include "map/homunculus.h"
#include "map/guild.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/mercenary.h"
#include "map/party.h"
#include "map/pet.h"
#include "map/pc.h"
#include "map/quest.h"

#include "plugins/HPMHooking.h"

#include "emap/clif.h"
#include "emap/lang.h"
#include "emap/map.h"
#include "emap/send.h"
#include "emap/data/itemd.h"
#include "emap/data/mapd.h"
#include "emap/data/session.h"
#include "emap/struct/itemdext.h"
#include "emap/struct/mapdext.h"
#include "emap/struct/sessionext.h"

#include <math.h>

extern bool isInit;
extern char global_npc_str[1001];

static inline void RBUFPOS(const uint8 *p,
                           unsigned short pos,
                           short *x,
                           short *y,
                           unsigned char *dir)
{
    p += pos;

    if (x)
    {
        x[0] = ((p[0] & 0xff) << 2) | (p[1] >> 6);
    }

    if (y)
    {
        y[0] = ((p[1] & 0x3f) << 4) | (p[2] >> 4);
    }

    if (dir)
    {
        dir[0] = (p[2] & 0x0f);
    }
}

static inline void RFIFOPOS(int fd,
                            unsigned short pos,
                            short *x,
                            short *y,
                            unsigned char *dir)
{
    RBUFPOS(RFIFOP(fd,pos), 0, x, y, dir);
}

void eclif_quest_send_list_pre(TBL_PC **sdPtr)
{
    TBL_PC *sd = *sdPtr;

    if (!sd)
    {
        hookStop();
        return;
    }

    int fd = sd->fd;
    int info_len = 4 + 1 + 3 * 4 + 4;
    int len = sd->avail_quests * info_len + 8;
    WFIFOHEAD(fd, len);
    WFIFOW(fd, 0) = 0xb23 + evolPacketOffset;
    WFIFOW(fd, 2) = len;
    WFIFOL(fd, 4) = sd->avail_quests;
    for (int i = 0; i < sd->avail_quests; i++ )
    {
        struct quest_db *qi = quest->db(sd->quest_log[i].quest_id);
        if (!qi)
            continue;
        WFIFOL(fd, i * info_len + 8) = sd->quest_log[i].quest_id;
        WFIFOB(fd, i * info_len + 12) = sd->quest_log[i].state;
        WFIFOL(fd, i * info_len + 13) = sd->quest_log[i].count[0];
        WFIFOL(fd, i * info_len + 17) = sd->quest_log[i].count[1];
        WFIFOL(fd, i * info_len + 21) = sd->quest_log[i].count[2];
        WFIFOL(fd, i * info_len + 25) = sd->quest_log[i].time;
    }
    WFIFOSET(fd, len);

    hookStop();
}

void eclif_quest_add_pre(TBL_PC **sdPtr,
                         struct quest **qdPtr)
{
    eclif_quest_add(*sdPtr, *qdPtr);
}

void eclif_quest_add(TBL_PC *sd,
                     struct quest *qd)
{
    if (!sd || !qd)
    {
        hookStop();
        return;
    }
    int fd = sd->fd;
    struct quest_db *qi = quest->db(qd->quest_id);

    if (!qi)
    {
        hookStop();
        return;
    }

    WFIFOHEAD(fd, 23);
    WFIFOW(fd, 0) = 0xb24 + evolPacketOffset;
    WFIFOL(fd, 2) = qd->quest_id;
    WFIFOB(fd, 6) = qd->state;
    WFIFOL(fd, 7) = qd->count[0];
    WFIFOL(fd, 11) = qd->count[1];
    WFIFOL(fd, 15) = qd->count[2];
    WFIFOL(fd, 19) = qd->time;
    WFIFOSET(fd, 23);

    hookStop();
}

// legacy eclif_blname_ack_legacy start
// clientVersion <= 24
//

unsigned int eget_percentage(const unsigned int A, const unsigned int B);
unsigned int eget_percentage(const unsigned int A, const unsigned int B)
{
    double result;

    if( B == 0 )
    {
        ShowError("get_percentage(): division by zero! (A=%u,B=%u)\n", A, B);
        return ~0U;
    }

    result = 100 * ((double)A / (double)B);

    if( result > UINT_MAX )
    {
        ShowError("get_percentage(): result percentage too high! (A=%u,B=%u,result=%g)\n", A, B, result);
        return UINT_MAX;
    }

    return (unsigned int)floor(result);
}

// ZC_ACK_REQNAMEALL / ZC_ACK_REQNAMEALL2
struct packet_reqnameall_legacy_ack {
    uint16 packet_id;
    int32 gid;
    char name[NAME_LENGTH];
    char party_name[NAME_LENGTH];
    char guild_name[NAME_LENGTH];
    char position_name[NAME_LENGTH];
} __attribute__((packed));

/// Updates the object's (bl) name on client.
/// 0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
/// 0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
/// 0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
static void eclif_blname_ack_legacy(int fd, struct block_list *bl)
{
    struct packet_reqnameall_legacy_ack packet;
    memset(&packet, 0, sizeof(struct packet_reqnameall_legacy_ack));
    int len = sizeof(struct packet_reqnameall_legacy_ack);

    nullpo_retv(bl);

    packet.packet_id = reqName;
    packet.gid = bl->id;

    switch(bl->type) {
        case BL_PC:
        {
            const struct map_session_data *ssd = BL_UCCAST(BL_PC, bl);
            const struct party_data *p = NULL;
            const struct guild *g = NULL;
            int ps = -1;

            if (ssd->fakename[0] != '\0' || ssd->status.guild_id > 0 || ssd->status.party_id > 0 || ssd->status.title_id > 0) {
                packet.packet_id = 0x195; //reqNameAllType;
            }

            //Requesting your own "shadow" name. [Skotlex]
            if (ssd->fd == fd && ssd->disguise != -1) {
                packet.gid = -bl->id;
            }

            if (ssd->fakename[0] != '\0') {
                memcpy(packet.name, ssd->fakename, NAME_LENGTH);
                break;
            }

//#if PACKETVER >= 20150503
//            // Title System [Dastgir/Hercules]
//            if (ssd->status.title_id > 0) {
//                packet.title_id = ssd->status.title_id;
//            }
//#endif

            memcpy(packet.name, ssd->status.name, NAME_LENGTH);

            if (ssd->status.party_id != 0) {
                p = party->search(ssd->status.party_id);
            }
            if (ssd->status.guild_id != 0) {
                if ((g = ssd->guild) != NULL) {
                    int i;
                    ARR_FIND(0, g->max_member, i, g->member[i].account_id == ssd->status.account_id && g->member[i].char_id == ssd->status.char_id);
                    if (i < g->max_member)
                        ps = g->member[i].position;
                }
            }

            if (!battle->bc->display_party_name && g == NULL) {
                // do not display party unless the player is also in a guild
                p = NULL;
            }

            if (p == NULL && g == NULL)
                break;

            if (p != NULL) {
                memcpy(packet.party_name, p->party.name, NAME_LENGTH);
            }

            if (g != NULL && ps >= 0 && ps < MAX_GUILDPOSITION) {
                memcpy(packet.guild_name, g->name,NAME_LENGTH);
                memcpy(packet.position_name, g->position[ps].name, NAME_LENGTH);
            }
        }
            break;
        //[blackhole89]
        case BL_HOM:
            memcpy(packet.name, BL_UCCAST(BL_HOM, bl)->homunculus.name, NAME_LENGTH);
            break;
        case BL_MER:
            memcpy(packet.name, BL_UCCAST(BL_MER, bl)->db->name, NAME_LENGTH);
            break;
        case BL_PET:
            memcpy(packet.name, BL_UCCAST(BL_PET, bl)->pet.name, NAME_LENGTH);
            break;
        case BL_NPC:
            memcpy(packet.name, BL_UCCAST(BL_NPC, bl)->name, NAME_LENGTH);
            break;
        case BL_MOB:
        {
            const struct mob_data *md = BL_UCCAST(BL_MOB, bl);

            memcpy(packet.name, md->name, NAME_LENGTH);
            if (md->guardian_data && md->guardian_data->g) {
                packet.packet_id = 0x195; //reqNameAllType;
                memcpy(packet.guild_name, md->guardian_data->g->name, NAME_LENGTH);
                memcpy(packet.position_name, md->guardian_data->castle->castle_name, NAME_LENGTH);

            } else if (battle->bc->show_mob_info) {
                char mobhp[50], *str_p = mobhp;
                packet.packet_id = 0x195;  //reqNameAllType;
                if (battle->bc->show_mob_info&4)
                    str_p += sprintf(str_p, "Lv. %d | ", md->level);
                if (battle->bc->show_mob_info&1)
                    str_p += sprintf(str_p, "HP: %u/%u | ", md->status.hp, md->status.max_hp);
                if (battle->bc->show_mob_info&2)
                    str_p += sprintf(str_p, "HP: %u%% | ", eget_percentage(md->status.hp, md->status.max_hp));
                //Even thought mobhp ain't a name, we send it as one so the client
                //can parse it. [Skotlex]
                if (str_p != mobhp) {
                    *(str_p-3) = '\0'; //Remove trailing space + pipe.
                    memcpy(packet.party_name, mobhp, NAME_LENGTH);
                }
            }
        }
            break;
        case BL_CHAT:
#if 0 //FIXME: Clients DO request this... what should be done about it? The chat's title may not fit... [Skotlex]
            memcpy(packet.name, BL_UCCAST(BL_CHAT, bl)->title, NAME_LENGTH);
            break;
#endif
            return;
        case BL_ELEM:
            memcpy(packet.name, BL_UCCAST(BL_ELEM, bl)->db->name, NAME_LENGTH);
            break;
        default:
            ShowError("clif_blname_ack: bad type %u(%d)\n", bl->type, bl->id);
            return;
    }

    if (packet.packet_id == reqName) {
        len = sizeof(struct packet_reqname_ack);
    }
    // if no recipient specified just update nearby clients
    // if no recipient specified just update nearby clients
    if (fd == 0) {
        clif->send(&packet, len, bl, AREA);
    } else {
        struct map_session_data *sd = sockt->session_is_valid(fd) ? sockt->session[fd]->session_data : NULL;
        if (sd != NULL) {
            clif->send(&packet, len, &sd->bl, SELF);
        } else {
            clif->send(&packet, len, bl, SELF);
        }
    }
}

//Used to update when a char leaves a party/guild. [Skotlex]
//Needed because when you send a 0x95 packet, the client will not remove the cached party/guild info that is not sent.
static void eclif_charnameupdate_legacy(struct map_session_data *ssd)
{
    int ps = -1;
    struct party_data *p = NULL;
    struct guild *g = NULL;
    struct packet_reqnameall_legacy_ack packet;
    memset(&packet, 0, sizeof(struct packet_reqnameall_legacy_ack));

    nullpo_retv(ssd);

    if (ssd->fakename[0])
        return; //No need to update as the party/guild was not displayed anyway.

    packet.packet_id = 0x195;  //reqNameAllType;
    packet.gid = ssd->bl.id;

    memcpy(packet.name, ssd->status.name, NAME_LENGTH);

    if (!battle->bc->display_party_name) {
        if (ssd->status.party_id > 0 && ssd->status.guild_id > 0 && (g = ssd->guild) != NULL)
            p = party->search(ssd->status.party_id);
    } else {
        if (ssd->status.party_id > 0)
            p = party->search(ssd->status.party_id);
    }

    if (ssd->status.guild_id > 0 && (g = ssd->guild) != NULL) {
        int i;
        ARR_FIND(0, g->max_member, i, g->member[i].account_id == ssd->status.account_id && g->member[i].char_id == ssd->status.char_id);
        if( i < g->max_member ) ps = g->member[i].position;
    }

    if (p != NULL)
        memcpy(packet.party_name, p->party.name, NAME_LENGTH);

    if (g != NULL && ps >= 0 && ps < MAX_GUILDPOSITION) {
        memcpy(packet.guild_name, g->name,NAME_LENGTH);
        memcpy(packet.position_name, g->position[ps].name, NAME_LENGTH);
    }

//#if PACKETVER >= 20150503
//    // Achievement System [Dastgir/Hercules]
//    if (ssd->status.title_id > 0) {
//        packet.title_id = ssd->status.title_id;
//    }
//#endif

    // Update nearby clients
    clif->send(&packet, sizeof(packet), &ssd->bl, AREA);
}

//
// clientVersion <= 24
// legacy eclif_blname_ack_legacy end

void eclif_blname_ack_pre(int *fdPtr,
                          struct block_list **blPtr)
{
    eclif_blname_ack_pre_sub(fdPtr, blPtr);
    if (hookStopped())
        return;

    struct SessionExt *data = session_get(*fdPtr);
    if (!data)
        return;
    if (data->clientVersion <= 24)
    {
        eclif_blname_ack_legacy(*fdPtr, *blPtr);
        hookStop();
    }
}

void eclif_blname_ack_pre_sub(int *fdPtr,
                              struct block_list **blPtr)
{
    struct block_list *bl = *blPtr;
    if (!bl)
    {
        hookStop();
        return;
    }
    if (bl->type == BL_NPC)
    {
        int fd = *fdPtr;
        TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
        if (!sd)
        {
            hookStop();
            return;
        }
        const char *tr = lang_pctrans(((TBL_NPC*)bl)->name, sd);
        const int trLen = (int)strlen(tr);
        const int len = 8 + trLen;
        // if no recipient specified just update nearby clients
        if (fd == 0)
        {
            char *buf;
            CREATE(buf, char, len);
            WBUFW(buf, 0) = 0xB01 + evolPacketOffset;
            WBUFW(buf, 2) = len;
            WBUFL(buf, 4) = bl->id;
            memcpy(WBUFP(buf, 8), tr, trLen);
            clif->send(buf, len, bl, AREA);
            aFree(buf);
        }
        else
        {
            WFIFOHEAD(fd, len);
            WFIFOW(fd, 0) = 0xB01 + evolPacketOffset;
            WFIFOW(fd, 2) = len;
            WFIFOL(fd, 4) = bl->id;
            memcpy(WFIFOP(fd, 8), tr, trLen);
            WFIFOSET(fd, len);
        }
        hookStop();
    }
    else if (bl->type == BL_MOB)
    {
        struct mob_data *md = (struct mob_data *)bl;
        if (!md)
        {
            hookStop();
            return;
        }
        if (md->guardian_data && md->guardian_data->g)
            return; // allow default code to work
        int fd = *fdPtr;
        TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
        if (!sd)
        {
            hookStop();
            return;
        }

        char tmpBuf[25];
        char *ptr = tmpBuf;
        int f;
        memcpy(tmpBuf, md->name, 24);
        tmpBuf[24] = 0;
        for (f = 23; f > 1; f --)
        {
            if (tmpBuf[f] == ' ')
                tmpBuf[f] = 0;
            else
                break;
        }
        for (f = 0; f < 24; f ++)
        {
            if (*ptr == ' ')
                ptr ++;
            else
                break;
        }
        const char *tr = lang_pctrans(ptr, sd);
        const int trLen = (int)strlen(tr);
        const int len = 8 + trLen;

        // if no recipient specified just update nearby clients
        if (fd == 0)
        {
            char *buf;
            CREATE(buf, char, len);
            WBUFW(buf, 0) = 0xB01 + evolPacketOffset;
            WBUFW(buf, 2) = len;
            WBUFL(buf, 4) = bl->id;
            memcpy(WBUFP(buf, 8), tr, trLen);
            clif->send(buf, len, bl, AREA);
            aFree(buf);
        }
        else
        {
            WFIFOHEAD(fd, len);
            WFIFOW(fd, 0) = 0xB01 + evolPacketOffset;
            WFIFOW(fd, 2) = len;
            WFIFOL(fd, 4) = bl->id;
            memcpy(WFIFOP(fd, 8), tr, trLen);
            WFIFOSET(fd, len);
        }
        hookStop();
    }
}

void eclif_charnameupdate_pre(struct map_session_data **ssdPtr)
{
    struct SessionExt *data = session_get_bysd(*ssdPtr);
    if (!data)
        return;
    if (data->clientVersion <= 24)
    {
        eclif_charnameupdate_legacy(*ssdPtr);
        hookStop();
    }
}

// clientVersion < 26
void eclif_homname_ack_pre(int *fdPtr, struct block_list **blPtr)
{
    struct block_list *bl = *blPtr;
    int fd = *fdPtr;
    struct SessionExt *data = session_get(fd);
    if (!data)
        return;
    if (data->clientVersion >= 26)
    {
        return;
    }

    nullpo_retv(bl);
    Assert_retv(bl->type == BL_HOM);

    struct packet_reqname_ack packet;
    memset(&packet, 0, sizeof(packet));
    packet.packet_id = reqName;
    packet.gid = bl->id;
    memcpy(packet.name, BL_UCCAST(BL_HOM, bl)->homunculus.name, NAME_LENGTH);
    clif->send_selforarea(fd, bl, &packet, sizeof(struct packet_reqname_ack));
    hookStop();
}

// clientVersion < 26
void eclif_mername_ack_pre(int *fdPtr, struct block_list **blPtr)
{
    struct block_list *bl = *blPtr;
    int fd = *fdPtr;
    struct SessionExt *data = session_get(fd);
    if (!data)
        return;
    if (data->clientVersion >= 26)
    {
        return;
    }

    nullpo_retv(bl);
    Assert_retv(bl->type == BL_MER);

    struct packet_reqname_ack packet;
    memset(&packet, 0, sizeof(packet));
    packet.packet_id = reqName;
    packet.gid = bl->id;
    memcpy(packet.name, BL_UCCAST(BL_MER, bl)->db->name, NAME_LENGTH);
    clif->send_selforarea(fd, bl, &packet, sizeof(struct packet_reqname_ack));
    hookStop();
}

// clientVersion < 26
void eclif_petname_ack_pre(int *fdPtr, struct block_list **blPtr)
{
    struct block_list *bl = *blPtr;
    int fd = *fdPtr;
    struct SessionExt *data = session_get(fd);
    if (!data)
        return;
    if (data->clientVersion >= 26)
    {
        return;
    }

    nullpo_retv(bl);
    Assert_retv(bl->type == BL_PET);

    struct packet_reqname_ack packet;
    memset(&packet, 0, sizeof(packet));
    packet.packet_id = reqName;
    packet.gid = bl->id;
    memcpy(packet.name, BL_UCCAST(BL_PET, bl)->pet.name, NAME_LENGTH);
    clif->send_selforarea(fd, bl, &packet, sizeof(struct packet_reqname_ack));
    hookStop();
}

// clientVersion < 26
void eclif_elemname_ack_pre(int *fdPtr, struct block_list **blPtr)
{
    struct block_list *bl = *blPtr;
    int fd = *fdPtr;
    struct SessionExt *data = session_get(fd);
    if (!data)
        return;
    if (data->clientVersion >= 26)
    {
        return;
    }

    nullpo_retv(bl);
    Assert_retv(bl->type == BL_ELEM);

    struct packet_reqname_ack packet;
    memset(&packet, 0, sizeof(packet));
    packet.packet_id = reqName;
    packet.gid = bl->id;
    memcpy(packet.name, BL_UCCAST(BL_ELEM, bl)->db->name, NAME_LENGTH);
    clif->send_selforarea(fd, bl, &packet, sizeof(struct packet_reqname_ack));
    hookStop();
}

#define equipPos(index, field) \
    equip = sd->equip_index[index]; \
    if (equip >= 0) \
    { \
        item = sd->inventory_data[equip]; \
        if (item && item->view_sprite) \
            send_changelook(sd, sd2, fd, id, field, item->view_sprite, 0, item, equip); \
    }

#define equipPosId(index, field) \
    equip = sd->equip_index[index]; \
    if (equip >= 0) \
    { \
        item = sd->inventory_data[equip]; \
        if (item && item->nameid) \
            send_changelook(sd, sd2, fd, id, field, item->nameid, 0, item, equip); \
    }

static void eclif_send_additional_slots(TBL_PC* sd, TBL_PC* sd2)
{
    if (!sd || !sd2)
        return;

    const int id = sd->bl.id;
    const int fd = sd2->fd;

    struct item_data *item;
    short equip;
    struct MapdExt *data = mapd_get(sd->bl.m);
    if (!data || data->invisible)
        return;

    equipPosId(EQI_HAND_R, LOOK_WEAPON);
    equipPosId(EQI_HAND_L, LOOK_SHIELD);
    equipPos(EQI_HEAD_LOW, LOOK_HEAD_BOTTOM);
    equipPos(EQI_HEAD_TOP, LOOK_HEAD_TOP);
    equipPos(EQI_HEAD_MID, LOOK_HEAD_MID);
    equipPos(EQI_GARMENT, LOOK_ROBE);
    equipPos(EQI_SHOES, LOOK_SHOES);
    equipPos(EQI_COSTUME_TOP, 13);
    equipPos(EQI_COSTUME_MID, 14);
    equipPos(EQI_COSTUME_LOW, 15);
    equipPos(EQI_COSTUME_GARMENT, 16);
    equipPos(EQI_ARMOR, 17);
    equipPos(EQI_ACC_R, 18);
    equipPos(EQI_ACC_L, 19);
    //skipping SHADOW slots
}

#undef equipPos
#undef equipPosId

#define equipPos2(index, field) \
    equip = sd->equip_index[index]; \
    if (equip >= 0) \
    { \
        item = sd->inventory_data[equip]; \
        if (item && item->view_sprite) \
        { \
            send_changelook2(sd, bl, bl->id, field, item->view_sprite, 0, item, equip, AREA); \
        } \
    }

#define equipPos2Id(index, field) \
    equip = sd->equip_index[index]; \
    if (equip >= 0) \
    { \
        item = sd->inventory_data[equip]; \
        if (item && item->nameid) \
        { \
            send_changelook2(sd, bl, bl->id, field, item->nameid, 0, item, equip, AREA); \
        } \
    }

static void eclif_send_additional_slots2(struct block_list *bl)
{
    if (!bl)
        return;

    TBL_PC* sd = (TBL_PC*)bl;
    struct item_data *item;
    short equip;
    struct MapdExt *data = mapd_get(sd->bl.m);
    if (!data || data->invisible)
        return;

    equipPos2Id(EQI_HAND_R, LOOK_WEAPON);
    equipPos2Id(EQI_HAND_L, LOOK_SHIELD);
    equipPos2(EQI_HEAD_LOW, LOOK_HEAD_BOTTOM);
    equipPos2(EQI_HEAD_TOP, LOOK_HEAD_TOP);
    equipPos2(EQI_HEAD_MID, LOOK_HEAD_MID);
    equipPos2(EQI_GARMENT, LOOK_ROBE);
    equipPos2(EQI_SHOES, LOOK_SHOES);
    equipPos2(EQI_COSTUME_TOP, 13);
    equipPos2(EQI_COSTUME_MID, 14);
    equipPos2(EQI_COSTUME_LOW, 15);
    equipPos2(EQI_COSTUME_GARMENT, 16);
    equipPos2(EQI_ARMOR, 17);
    equipPos2(EQI_ACC_R, 18);
    equipPos2(EQI_ACC_L, 19);
    //skipping SHADOW slots
}

#undef equipPos2
#undef equipPos2Id

void eclif_getareachar_unit_post(TBL_PC *sd,
                                 struct block_list *bl)
{
    if (!bl || !sd)
        return;
    if (bl->type == BL_PC)
    {
        eclif_send_additional_slots((TBL_PC *)bl, sd);
        send_pc_info(bl, &sd->bl, SELF);
    }
}

bool eclif_spawn_post(bool retVal,
                      struct block_list *bl)
{
    if (!bl)
        return retVal;
    if (retVal == true && bl->type == BL_PC)
    {
        send_pc_info(bl, bl, AREA);
        eclif_send_additional_slots2(bl);
    }
    return retVal;
}

void eclif_authok_post(TBL_PC *sd)
{
    if (!sd)
        return;

    eclif_send_additional_slots(sd, sd);
    send_pc_info(&sd->bl, &sd->bl, SELF);
    send_pc_own_flags(&sd->bl);
    struct MapdExt *data = mapd_get(sd->bl.m);
    int mask = data ? data->mask : 1;
    send_mapmask(sd->fd, mask);
}

void eclif_changemap_post(TBL_PC *sd,
                          short m,
                          int x __attribute__ ((unused)),
                          int y __attribute__ ((unused)))
{
    if (!sd)
        return;
    struct MapdExt *data = mapd_get(m);
    int mask = data ? data->mask : 1;
    send_mapmask(sd->fd, mask);
}

void eclif_handle_invisible_map(struct block_list *bl,
                                enum send_target target __attribute__ ((unused)))
{
    if (!bl || bl->type != BL_PC)
        return;
    struct MapdExt *data = mapd_get(bl->m);
    if (data && data->invisible)
        hookStop();
}

void eclif_sendlook_pre(struct block_list **blPtr,
                        int *id __attribute__ ((unused)),
                        int *type __attribute__ ((unused)),
                        int *val __attribute__ ((unused)),
                        int *val2 __attribute__ ((unused)),
                        enum send_target *target)
{
    struct block_list *bl = *blPtr;
    if (*target == SELF)
        return;
    eclif_handle_invisible_map(bl, *target);
}

bool eclif_send_pre(const void **bufPtr __attribute__ ((unused)),
                    int *len __attribute__ ((unused)),
                    struct block_list **blPtr,
                    enum send_target *type)
{
    struct block_list *bl = *blPtr;
    if (*type == SELF)
    {
        return true;
    }
    eclif_handle_invisible_map(bl, *type);
    return true;
}

void eclif_set_unit_idle_pre(struct block_list **blPtr,
                             TBL_PC **tsdPtr,
                             enum send_target *target)
{
    struct block_list *bl = *blPtr;
    TBL_PC *tsd = *tsdPtr;
    if (tsd && bl && bl->id == tsd->bl.id && *target == SELF)
        return;

    eclif_handle_invisible_map(bl, *target);
}

int eclif_send_actual_pre(int *fd __attribute__ ((unused)),
                          void **bufPtr,
                          int *len)
{
    void *buf = *bufPtr;
    if (*len >= 2)
    {
        const int packet = RBUFW (buf, 0);
        if (packet == 0x1d7)
        {
            // not sending old packets to new clients
            // probably useless
            hookStop();
            return 0;
        }
        if (packet == 0x7fb)
        {
            // not sending old packets to new clients
            // probably useless?
            hookStop();
            return 0;
        }
    }
    return 0;
}

uint16 GetWord(uint32 val, int idx)
{
    switch( idx )
    {
        case 0: return (uint16)( (val & 0x0000FFFF)         );
        case 1: return (uint16)( (val & 0xFFFF0000) >> 0x10 );
        default:
#if defined(DEBUG)
            ShowDebug("GetWord: invalid index (idx=%d)\n", idx);
#endif
            return 0;
    }
}

void eclif_set_unit_idle_post(struct block_list *bl,
                              TBL_PC *tsd,
                              enum send_target target)
{
    if (!bl || !tsd)
        return;

    if (bl->type == BL_MOB)
        send_mob_info(bl, &tsd->bl, target);
    else if (bl->type == BL_PC)
        send_pc_info(bl, &tsd->bl, target);
    else if (bl->type == BL_NPC)
        send_npc_info(bl, &tsd->bl, target);
}

void eclif_set_unit_walking_post(struct block_list *bl,
                                 TBL_PC *tsd,
                                 struct unit_data* ud,
                                 enum send_target target)
{
    if (!ud)
        return;
    TBL_PC *sd = BL_CAST(BL_PC, ud->bl);
    if (!sd || !pc_isinvisible(sd))
    {
        if (ud->walktimer != INVALID_TIMER)
            send_advmoving(ud, true, tsd ? &tsd->bl : bl, target);
        else
            send_advmoving(ud, false, tsd ? &tsd->bl : bl, target);
    }
}

void eclif_move_post(struct unit_data *ud)
{
    if (!ud)
        return;
    TBL_PC *sd = BL_CAST(BL_PC, ud->bl);
    if (!sd || !pc_isinvisible(sd))
        send_advmoving(ud, false, ud->bl, AREA_WOS);
}

bool tempChangeMap;

void eclif_parse_LoadEndAck_pre(int *fdPtr __attribute__ ((unused)),
                                struct map_session_data **sdPtr)
{
    struct map_session_data *sd = *sdPtr;
    if (!sd)
        return;
    sd->state.warp_clean = 0;
    tempChangeMap = sd->state.changemap;
}

void eclif_parse_LoadEndAck_post(int fd __attribute__ ((unused)),
                                 struct map_session_data *sd)
{
    if (!tempChangeMap)
    {   // some messages not sent if map not changed
        map->iwall_get(sd);
    }
    map_alwaysVisible_send(sd);
}

void eclif_changelook2(struct block_list *bl,
                       int type,
                       int val,
                       struct item_data *id,
                       int n)
{
    struct map_session_data* sd;
    struct status_change* sc;
    struct view_data* vd;
    enum send_target target = AREA;
    int val2 = 0;
    if (!bl)
        return;

    sd = BL_CAST(BL_PC, bl);
    sc = status->get_sc(bl);
    vd = status->get_viewdata(bl);

    if (vd)
    {
        switch(type)
        {
            case LOOK_WEAPON:
                if (sd)
                {
                    clif->get_weapon_view(sd, &vd->weapon, &vd->shield);
                    val = vd->weapon;
                }
                else
                {
                    vd->weapon = val;
                }
                break;

            case LOOK_SHIELD:
                if (sd)
                {
                    clif->get_weapon_view(sd, &vd->weapon, &vd->shield);
                    val = vd->shield;
                }
                else
                {
                    vd->shield = val;
                }
                break;
            case LOOK_BASE:
                if (!sd)
                    break;

                if (val == INVISIBLE_CLASS) /* nothing to change look */
                    return;

                if (sd->sc.option & OPTION_COSTUME)
                    vd->weapon = vd->shield = 0;

//                if (!vd->cloth_color)
//                    break;
            break;
            case LOOK_HAIR:
                vd->hair_style = val;
            break;
            case LOOK_HEAD_BOTTOM:
                vd->head_bottom = val;
            break;
            case LOOK_HEAD_TOP:
                vd->head_top = val;
            break;
            case LOOK_HEAD_MID:
                vd->head_mid = val;
            break;
            case LOOK_HAIR_COLOR:
                vd->hair_color = val;
            break;
            case LOOK_CLOTHES_COLOR:
                vd->cloth_color = val;
            break;
            case LOOK_SHOES:
//                if (sd) {
//                    int n;
//                    if((n = sd->equip_index[2]) >= 0 && sd->inventory_data[n]) {
//                        if(sd->inventory_data[n]->view_id > 0)
//                            val = sd->inventory_data[n]->view_id;
//                        else
//                            val = sd->status.inventory[n].nameid;
//                    } else
//                        val = 0;
//                }
            break;
            case LOOK_BODY:
            case LOOK_FLOOR:
                // unknown purpose
            break;
            case LOOK_ROBE:
                vd->robe = val;
            break;
        }
    }

    // prevent leaking the presence of GM-hidden objects
    if (sc && sc->option&OPTION_INVISIBLE && !( bl->type == BL_PC && ((TBL_PC*)bl)->disguise != -1))
        target = SELF;
    if (type == LOOK_WEAPON || type == LOOK_SHIELD)
    {
        if (!vd)
            return;
        type = LOOK_WEAPON;
        val = vd->weapon;
        val2 = vd->shield;
    }
    if ((bl->type == BL_PC && ((TBL_PC*)bl)->disguise != -1))
    {
        send_changelook2(sd, bl, bl->id, type, val, val2, id, n, AREA_WOS);
        send_changelook2(sd, bl, -bl->id, type, val, val2, id, n, SELF);
    }
    else
    {
        send_changelook2(sd, bl, bl->id, type, val, val2, id, n, target);
    }
}

static inline int itemtype(const int type)
{
    switch (type)
    {
#if PACKETVER >= 20080827
        case IT_WEAPON:
            return IT_ARMOR;
        case IT_ARMOR:
        case IT_PETARMOR:
#endif
        case IT_PETEGG:
            return IT_WEAPON;
        default:
            return type;
    }
}

void eclif_getareachar_item_pre(struct map_session_data **sdPtr,
                                struct flooritem_data **fitemPtr)
{
    int view;
    struct map_session_data *sd = *sdPtr;
    struct flooritem_data *fitem = *fitemPtr;
    if (!sd || !fitem)
        return;
    int fd = sd->fd;

    struct SessionExt *data = session_get(fd);
    if (!data)
        return;

    WFIFOHEAD(fd, 28);
    WFIFOW(fd, 0) = 0xb18 + evolPacketOffset;
    WFIFOL(fd, 2) = fitem->bl.id;
    if((view = itemdb_viewid(fitem->item_data.nameid)) > 0)
        WFIFOW(fd, 6) = view;
    else
        WFIFOW(fd, 6) = fitem->item_data.nameid;
    WFIFOB(fd, 8) = itemtype(itemdb_type(fitem->item_data.nameid));
    WFIFOB(fd, 9) = fitem->item_data.identify;
    WFIFOB(fd, 10) = fitem->item_data.attribute;
    WFIFOB(fd, 11) = fitem->item_data.refine;
    clif->addcards((struct EQUIPSLOTINFO*)WFIFOP(fd, 12), &fitem->item_data);
    WFIFOW(fd, 20) = fitem->bl.x;
    WFIFOW(fd, 22) = fitem->bl.y;
    WFIFOW(fd, 24) = fitem->item_data.amount;
    WFIFOB(fd, 26) = fitem->subx;
    WFIFOB(fd, 27) = fitem->suby;
    WFIFOSET(fd, 28);
    hookStop();
}

void eclif_dropflooritem_pre(struct flooritem_data **fitemPtr)
{
    char buf[28];
    int view;
    struct flooritem_data *fitem = *fitemPtr;

    if (!fitem)
    {
        hookStop();
        return;
    }
    struct ItemdExt *itemData = itemd_get_by_item(&fitem->item_data);
    if (itemData)
    {
        if (itemData->subX)
            fitem->subx = (rand() % (itemData->subX * 2)) - itemData->subX;
        else
            fitem->subx = 0;
        if (itemData->subY)
            fitem->suby = (rand() % (itemData->subY * 2)) - itemData->subY;
        else
            fitem->suby = 0;
    }

    WBUFW(buf, 0) = 0xb19 + evolPacketOffset;
    WBUFL(buf, 2) = fitem->bl.id;
    if((view = itemdb_viewid(fitem->item_data.nameid)) > 0)
        WBUFW(buf, 6) = view;
    else
        WBUFW(buf, 6) = fitem->item_data.nameid;
    WBUFB(buf, 8) = itemtype(itemdb_type(fitem->item_data.nameid));
    WBUFB(buf, 9) = fitem->item_data.identify;
    WBUFB(buf, 10) = fitem->item_data.attribute;
    WBUFB(buf, 11) = fitem->item_data.refine;
    clif->addcards((struct EQUIPSLOTINFO*)WBUFP(buf, 12), &fitem->item_data);
    WBUFW(buf, 20) = fitem->bl.x;
    WBUFW(buf, 22) = fitem->bl.y;
    WBUFW(buf, 24) = fitem->item_data.amount;
    WBUFB(buf, 26) = fitem->subx;
    WBUFB(buf, 27) = fitem->suby;

    clif->send(&buf, 28, &fitem->bl, AREA);
    hookStop();
}

void eclif_sendbgemblem_area_pre(struct map_session_data **sdPtr)
{
    unsigned char buf[34];
    struct map_session_data *sd = *sdPtr;
    struct SessionExt *data = session_get_bysd(sd);
    if (!sd || !data)
        return;

    WBUFW(buf, 0) = 0xb1a + evolPacketOffset;
    WBUFL(buf, 2) = sd->bl.id;
    safestrncpy(WBUFP(buf,6), sd->status.name, NAME_LENGTH); // name don't show in screen.
    WBUFW(buf, 30) = sd->bg_id;
    WBUFW(buf, 32) = data->teamId;
    clif->send(buf, 34, &sd->bl, AREA);
    hookStop();
}

void eclif_sendbgemblem_single_pre(int *fdPtr,
                                   struct map_session_data **sdPtr)
{
    int fd = *fdPtr;
    struct map_session_data *sd = *sdPtr;
    struct SessionExt *data = session_get_bysd(sd);
    struct SessionExt *ddata = session_get_bysd(sd);
    if (!sd || !data || !ddata)
    {
        hookStop();
        return;
    }

    WFIFOHEAD(fd, 34);
    WFIFOW(fd, 0) = 0xb1a + evolPacketOffset;
    WFIFOL(fd, 2) = sd->bl.id;
    safestrncpy(WFIFOP(fd, 6), sd->status.name, NAME_LENGTH);
    WFIFOW(fd, 30) = sd->bg_id;
    WFIFOW(fd, 32) = data->teamId;
    WFIFOSET(fd, 34);
    hookStop();
    return;
}

void eclif_disp_message_pre(struct block_list **srcPtr,
                            const char **mesPtr,
                            enum send_target *targetPtr)
{
    unsigned char buf[256];
    struct block_list *src = *srcPtr;
    const char *mes = *mesPtr;

    nullpo_retv(mes);

    int len = (int)strlen(mes);

    if (len == 0 || !isInit)
        return;

    nullpo_retv(src);

    if (len > sizeof(buf) - 5)
    {
        ShowWarning("clif_disp_message: Truncated message '%s' (len=%d, max=%d, aid=%d).\n", mes, (int)len, (int)(sizeof(buf) - 5), src->id);
        len = sizeof(buf) - 5;
    }

    WBUFW(buf, 0) = 0x8e;
    WBUFW(buf, 2) = len + 5;
    safestrncpy(WBUFP(buf, 4), mes, len + 1);
    clif->send(buf, WBUFW(buf, 2), src, *targetPtr);
}

void eclif_addcards_post(struct EQUIPSLOTINFO *buf,
                         struct item *item)
{
    if (!buf || !item)
        return;
    if (item->card[0] == CARD0_PET)
    {
        buf->card[0] = item->card[0];
        buf->card[1] = item->card[1];
        buf->card[2] = item->card[2];
        buf->card[3] = item->card[3];
    }
}

void eclif_useskill(struct block_list* bl,
                    int src_id,
                    int dst_id,
                    int dst_x,
                    int dst_y,
                    uint16 skill_id,
                    uint16 skill_lv,
                    int casttime)
{
    const int cmd = 0x7fb;
    unsigned char buf[50];
    int property = skill->get_ele(skill_id, skill_lv);

    // for client < 18
    WBUFW(buf, 0) = cmd;
    WBUFL(buf, 2) = src_id;
    WBUFL(buf, 6) = dst_id;
    WBUFW(buf, 10) = dst_x;
    WBUFW(buf, 12) = dst_y;
    WBUFW(buf, 14) = skill_id;
    WBUFL(buf, 16) = property < 0 ? 0 : property; //Avoid sending negatives as element [Skotlex]
    WBUFL(buf, 20) = casttime;
    WBUFB(buf, 24) = 0;  // isDisposable

    if (clif->isdisguised(bl))
    {
        clif->send(buf, 25, bl, AREA_WOS);
        WBUFL(buf, 2) = -src_id;
        clif->send(buf, 25, bl, SELF);
    }
    else
    {
        clif->send(buf, 25, bl, AREA);
    }

    // for client >= 18
    const int len = 36;
    WBUFW(buf, 0) = 0xb1e + evolPacketOffset;
    WBUFW(buf, 2) = len;
    WBUFL(buf, 4) = src_id;
    WBUFL(buf, 8) = dst_id;
    WBUFW(buf, 12) = dst_x;
    WBUFW(buf, 14) = dst_y;
    WBUFW(buf, 16) = skill_id;
    WBUFW(buf, 18) = skill_lv;
    WBUFL(buf, 20) = property < 0 ? 0 : property; //Avoid sending negatives as element [Skotlex]
    WBUFL(buf, 24) = casttime;
    WBUFL(buf, 28) = skill->get_splash(skill_id, skill_lv);
    WBUFL(buf, 32) = skill->get_inf2(skill_id);

    if (clif->isdisguised(bl))
    {
        clif->send(buf, len, bl, AREA_WOS);
        WBUFL(buf, 2) = -src_id;
        clif->send(buf, len, bl, SELF);
    }
    else
    {
        clif->send(buf, len, bl, AREA);
    }
#if PACKETVER >= 20151223
   if ((skill->get_inf2(skill_id) & INF2_SHOW_SKILL_SCALE) != 0)
       clif->skill_scale(bl, src_id, bl->x, bl->y, skill_id, skill_lv, casttime);
#endif
}

void eclif_skillinfoblock_pre(struct map_session_data **sdPtr)
{
    struct map_session_data *sd = *sdPtr;
    nullpo_retv(sd);

    int fd = sd->fd;
    if (!fd)
        return;

    WFIFOHEAD(fd, MAX_SKILL_DB * 41 + 4);
    WFIFOW(fd, 0) = 0x10f;
    int len = 4;
    int i;
    for (i = 0; i < MAX_SKILL_DB; i++)
    {
        int id = sd->status.skill[i].id;
        if (id != 0)
        {
            const int skillSize = 41;
            if (len + skillSize > 16384)
                break;

            WFIFOW(fd, len) = id;
            WFIFOL(fd, len + 2) = skill->get_inf(id);
            WFIFOL(fd, len + 6) = skill->get_inf2(id);
            const int level = sd->status.skill[i].lv;
            WFIFOW(fd, len + 10) = level;
            if (level)
            {
                WFIFOW(fd, len + 12) = skill->get_sp(id, level);
                WFIFOW(fd, len + 14) = skill->get_range2(&sd->bl, id, level);
            }
            else
            {
                WFIFOW(fd, len + 12) = 0;
                WFIFOW(fd, len + 14) = 0;
            }
            safestrncpy(WFIFOP(fd, len + 16), skill->get_name(id), NAME_LENGTH);
            if (sd->status.skill[i].flag == SKILL_FLAG_PERMANENT)
                WFIFOB(fd, len + 40) = (sd->status.skill[i].lv < skill->tree_get_max(id, sd->status.class)) ? 1 : 0;
            else
                WFIFOB(fd, len + 40) = 0;
            len += skillSize;
        }
    }
    WFIFOW(fd, 2) = len;
    WFIFOSET(fd, len);

    // workaround for bugreport:5348; send the remaining skills one by one to bypass packet size limit
    for ( ; i < MAX_SKILL_DB; i++)
    {
        int id = sd->status.skill[i].id;
        if (id != 0)
        {
            clif->addskill(sd, id);
            clif->skillinfo(sd, id, 0);
        }
    }
    hookStop();
}

void eclif_addskill_pre(struct map_session_data **sdPtr,
                        int *idPtr)
{
    struct map_session_data *sd = *sdPtr;
    nullpo_retv(sd);
    int id = *idPtr;
    int fd = sd->fd;
    if (!fd)
    {
        hookStop();
        return;
    }

    const int idx = skill->get_index(id);

    if (sd->status.skill[idx].id <= 0)
    {
        hookStop();
        return;
    }

    const int skill_lv = sd->status.skill[idx].lv;
    const int sz = 45;

    WFIFOHEAD(fd, sz);
    WFIFOW(fd, 0) = 0xb1f + evolPacketOffset;
    WFIFOW(fd, 2) = sz;
    WFIFOW(fd, 4) = id;
    WFIFOL(fd, 6) = skill->get_inf(id);
    WFIFOL(fd, 10) = skill->get_inf2(id);
    WFIFOW(fd, 14) = skill_lv;
    if (skill_lv > 0)
    {
        WFIFOW(fd, 16) = skill->get_sp(id, skill_lv);
        WFIFOW(fd, 18) = skill->get_range2(&sd->bl, id, skill_lv);
    }
    else
    {
        WFIFOW(fd, 16) = 0;
        WFIFOW(fd, 18) = 0;
    }
    safestrncpy(WFIFOP(fd, 20), skill->get_name(id), NAME_LENGTH);
    if (sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT)
        WFIFOB(fd, 44) = (skill_lv < skill->tree_get_max(id, sd->status.class)) ? 1 : 0;
    else
        WFIFOB(fd, 44) = 0;
    WFIFOSET(fd, sz);

    hookStop();
}

void eclif_skillinfo_pre(struct map_session_data **sdPtr,
                         int *skill_idPtr,
                         int *infPtr)
{
    struct map_session_data *sd = *sdPtr;
    nullpo_retv(sd);

    int skill_id = *skill_idPtr;
    int idx = skill->get_index(skill_id);
    Assert_retv(idx >= 0 && idx < MAX_SKILL_DB);
    int inf = *infPtr;

    const int fd = sd->fd;
    int skill_lv = sd->status.skill[idx].lv;

    const int sz = 21;
    WFIFOHEAD(fd, sz);
    WFIFOW(fd, 0) = 0xb20 + evolPacketOffset;
    WFIFOW(fd, 2) = sz;
    WFIFOW(fd, 4) = skill_id;
    WFIFOL(fd, 6) = inf ? inf : skill->get_inf(skill_id);
    WFIFOL(fd, 10) = skill->get_inf2(skill_id);
    WFIFOW(fd, 14) = skill_lv;
    if (skill_lv > 0) {
        WFIFOW(fd, 16) = skill->get_sp(skill_id, skill_lv);
        WFIFOW(fd, 18) = skill->get_range2(&sd->bl, skill_id, skill_lv);
    } else {
        WFIFOW(fd, 16) = 0;
        WFIFOW(fd, 18) = 0;
    }
    if (sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT)
        WFIFOB(fd, 20) = (skill_lv < skill->tree_get_max(skill_id, sd->status.class)) ? 1 : 0;
    else
        WFIFOB(fd, 20) = 0;
    WFIFOSET(fd, sz);
}

void eclif_parse_WalkToXY(int fd,
                          struct map_session_data *sd)
{
    short x, y;

    if (pc_isdead(sd))
    {
        clif->clearunit_area(&sd->bl, CLR_DEAD);
        return;
    }

    if (sd->sc.opt1 && (sd->sc.opt1 == OPT1_STONEWAIT || sd->sc.opt1 == OPT1_BURNING))
        ; //You CAN walk on this OPT1 value.
    /*else if( sd->progressbar.npc_id )
        clif->progressbar_abort(sd);*/
    else if (pc_cant_act(sd))
        return;

    if(sd->sc.data[SC_RUN] || sd->sc.data[SC_WUGDASH])
        return;

    pc->delinvincibletimer(sd);

    RFIFOPOS(fd, 2, &x, &y, NULL);

    //Set last idle time... [Skotlex]
    pc->update_idle_time(sd, BCIDLE_WALK);

    if (sd->ud.state.change_walk_target == 0)
    {
        if (unit->walk_toxy(&sd->bl, x, y, 4) == 0 &&
            sd->ud.state.change_walk_target == 1)
        {
            send_walk_fail(sd->fd, x, y);
        }
    }
    else
    {
        unit->walk_toxy(&sd->bl, x, y, 4);
    }
}

void eclif_party_info_post(struct party_data *p,
                           struct map_session_data *sd)
{
    if (sd)
    {
        clif->party_option(p, sd, 2);
    }
}

/// NPC text input dialog value (CZ_INPUT_EDITDLGSTR).
/// 01d5 <packet len>.W <npc id>.L <string>.?B
void eclif_parse_NpcStringInput(int fd,
                                struct map_session_data* sd)
{
// [4144] can't confirm exact client version. At least >= correct for 20150513
#if PACKETVER >= 20151029
   int message_len = RFIFOW(fd, 2) - 7;
#else
   int message_len = RFIFOW(fd, 2) - 8;
#endif
    int npcid = RFIFOL(fd, 4);
    const char *message = RFIFOP(fd, 8);

    if (message_len <= 0)
        return; // invalid input

    if (message_len > 1000)
        message_len = 1000;

    safestrncpy(global_npc_str, message, message_len);
    npc->scriptcont(sd, npcid, false);
}

void eclif_rodex_icon_pre(int *fdPtr,
                          bool *showPtr __attribute__ ((unused)))
{
    struct map_session_data *sd = sockt->session[*fdPtr]->session_data;
    struct SessionExt *data = session_get_bysd(sd);
    if (!data)
        return;
    if (data->clientVersion < 23)
    {
        hookStop();
        return;
    }
}

void eclif_force_charselect(struct map_session_data *sd)
{
	int fd = sd->fd;

    /* Rovert's Prevent logout option - Fixed [Valaris] */
    if (!sd->sc.data[SC_CLOAKING] && !sd->sc.data[SC_HIDING] && !sd->sc.data[SC_CHASEWALK]
        && !sd->sc.data[SC_CLOAKINGEXCEED] && !sd->sc.data[SC__INVISIBILITY] && !sd->sc.data[SC_SUHIDE]
        && (!battle->bc->prevent_logout || DIFF_TICK(timer->gettick(), sd->canlog_tick) > battle->bc->prevent_logout)
    ) {
        //Send to char-server for character selection.
        chrif->charselectreq(sd, sockt->session[fd]->client_addr);
    } else {
        // GM-kick the player
        clif->GM_kick(NULL, sd);
    }
}
