// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/channel.h"
#include "map/clif.h"
#include "map/homunculus.h"
#include "map/mercenary.h"
#include "map/pc.h"
#include "map/pet.h"
#include "map/unit.h"

#include "emap/parse.h"
#include "emap/send.h"
#include "emap/map.h"
#include "emap/data/itemd.h"
#include "emap/data/session.h"
#include "emap/struct/itemdext.h"
#include "emap/struct/sessionext.h"

void map_parse_version(int fd)
{
    struct SessionExt *data = session_get(fd);
    if (!data)
        return;
    data->clientVersion = RFIFOL(fd, 2);
}

void map_parse_join_channel(int fd)
{
    char name[24];
    char *p;
    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    int res = 0;
    if (!sd || !sd->bl.prev)
        return;

    safestrncpy(name, RFIFOP(fd, 2), 24);
    if (name[0] == '#')
        p = name + 1;
    else
        p = name;

    struct channel_data *chan = channel->search(p, sd);

    if (chan)
    {
        int k;
        ARR_FIND(0, VECTOR_LENGTH(sd->channels), k, VECTOR_INDEX(sd->channels, k) == chan);
        if (k < VECTOR_LENGTH(sd->channels) || channel->join(chan, sd, "", true) == HCS_STATUS_OK)
            res = 1;
        else
            res = 0;
    }

    send_join_ack(fd, name, res);
}

void map_parse_part_channel(int fd)
{
    char name[24];
    char *p;
    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    int k;
    if (!sd || !sd->bl.prev)
        return;

    safestrncpy(name, RFIFOP(fd, 2), 24);
    if (name[0] == '#')
        p = name + 1;
    else
        p = name;

    ARR_FIND(0, VECTOR_LENGTH(sd->channels), k, strcmpi(p, VECTOR_INDEX(sd->channels, k)->name) == 0);
    if (k == VECTOR_LENGTH(sd->channels))
    {
        return;
    }

    if (VECTOR_INDEX(sd->channels, k)->type == HCS_TYPE_ALLY)
    {
        for (k = VECTOR_LENGTH(sd->channels) - 1; k >= 0; k--)
        {
            // Loop downward to avoid issues when channel->leave() compacts the array
            if (VECTOR_INDEX(sd->channels, k)->type == HCS_TYPE_ALLY)
            {
                channel->leave(VECTOR_INDEX(sd->channels, k), sd);
            }
        }
    }
    else
    {
        channel->leave(VECTOR_INDEX(sd->channels, k), sd);
    }
}

void map_parse_pet_say(int fd)
{
    char message[500];

    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd || !sd->pd || !sd->bl.prev)
        return;

    if (!pc->can_talk(sd))
        return;

    const int len = RFIFOW(fd, 2);
    if (len > 500 || len < 6)
        return;
    safestrncpy(message, RFIFOP(fd, 4), len - 4);
    send_slave_say(sd, &sd->pd->bl, sd->pd->pet.name, message);
}

void map_parse_pet_emote(int fd)
{
    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd || !sd->pd || !sd->bl.prev)
        return;
    const time_t t = time(NULL);
    if (sd->emotionlasttime + 1 >= t)
    { // not more than 1 per second
        sd->emotionlasttime = t;
        return;
    }

    sd->emotionlasttime = t;
    clif->emotion(&sd->pd->bl, RFIFOB(fd, 2));
}

void map_parse_set_status(int fd)
{
    struct SessionExt *data = session_get(fd);
    if (!data)
        return;
    data->state = RFIFOB(fd, 2);
}

void map_parse_get_online_list(int fd)
{
    emap_online_list(fd);
}

void map_parse_pet_move(int fd)
{
    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd || !sd->pd || !sd->bl.prev)
        return;
    short x = RFIFOW(fd, 6);
    short y = RFIFOW(fd, 8);

    struct block_list *pdBl = &sd->pd->bl;
    if (map->getcell(pdBl->m, pdBl, x, y, CELL_CHKPASS))
        unit->walktoxy(pdBl, x, y, 0);
}

void map_parse_pet_dir(int fd)
{
    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd || !sd->pd || !sd->bl.prev)
        return;
    unit->setdir(&sd->pd->bl, RFIFOB(fd, 8));
}

void map_parse_homun_say(int fd)
{
    char message[500];

    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd || !sd->bl.prev)
        return;
    if (!pc->can_talk(sd))
        return;
    const int len = RFIFOW(fd, 2);
    if (len > 500 || len < 6)
        return;
    safestrncpy(message, RFIFOP(fd, 4), len - 4);
    if (sd->md && sd->md->db)
        send_slave_say(sd, &sd->md->bl, sd->md->db->name, message);
    else if (sd->hd && homun_alive(sd->hd))
        send_slave_say(sd, &sd->hd->bl, sd->hd->homunculus.name, message);
}

void map_parse_homun_emote(int fd)
{
    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd || !sd->bl.prev)
        return;
    const time_t t = time(NULL);
    if (sd->emotionlasttime + 1 >= t)
    { // not more than 1 per second
        sd->emotionlasttime = t;
        return;
    }

    sd->emotionlasttime = t;
    if (sd->md && sd->md->db)
        clif->emotion(&sd->md->bl, RFIFOB(fd, 2));
    else if (sd->hd && homun_alive(sd->hd))
        clif->emotion(&sd->hd->bl, RFIFOB(fd, 2));
}

void map_parse_homun_dir(int fd)
{
    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd || !sd->bl.prev)
        return;
    if (sd->md && sd->md->db)
        unit->setdir(&sd->md->bl, RFIFOB(fd, 8));
    else if (sd->hd && homun_alive(sd->hd))
        unit->setdir(&sd->hd->bl, RFIFOB(fd, 8));
}

void map_clif_parse_useitem2(int fd)
{
    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd || !sd->bl.prev)
        return;

    if (pc_isdead(sd))
    {
        clif->clearunit_area(&sd->bl, CLR_DEAD);
        return;
    }

    if ((!sd->npc_id && pc_istrading(sd)) || sd->chat_id != 0)
        return;

    // Whether the item is used or not is irrelevant, the char ain't idle. [Skotlex]
    pc->update_idle_time(sd, BCIDLE_USEITEM);
    const int n = RFIFOW(fd, 2) - 2;
    if (n < 0 || n >= MAX_INVENTORY)
        return;

    struct item_data *item = itemdb->exists(sd->inventory_data[n]->nameid);

    if (!item)
        return;
    struct ItemdExt *data = itemd_get(item);
    if (!data)
        return;

    data->tmpUseType = RFIFOW(fd, 4);
    if (!pc->useitem(sd, n))
        clif->useitemack(sd, n, 0, false); //Send an empty ack packet or the client gets stuck.
}
