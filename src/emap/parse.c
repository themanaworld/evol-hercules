// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/malloc.h"
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
#include "emap/data/session.h"
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
    if (!sd)
        return;

    safestrncpy(name, (char*)RFIFOP(fd, 2), 24);
    if (name[0] == '#')
        p = name + 1;
    else
        p = name;

    struct channel_data *chan = channel->search(p, sd);

    if (chan)
    {
        int k;
        ARR_FIND(0, sd->channel_count, k, sd->channels[k] == chan);
        if (k < sd->channel_count || channel->join(chan, sd, NULL, true) == HCS_STATUS_OK)
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
    if (!sd)
        return;

    safestrncpy(name, (char*)RFIFOP(fd, 2), 24);
    if (name[0] == '#')
        p = name + 1;
    else
        p = name;

    for (k = 0; k < sd->channel_count; k ++)
    {
        if (strcmpi(p, sd->channels[k]->name) == 0)
            break;
    }

    if (k == sd->channel_count)
        return;

    if (sd->channels[k]->type == HCS_TYPE_ALLY)
    {
        do
        {
            for (k = 0; k < sd->channel_count; k++)
            {
                if (sd->channels[k]->type == HCS_TYPE_ALLY)
                {
                    channel->leave(sd->channels[k], sd);
                    break;
                }
            }
        }
        while (k != sd->channel_count);
    }
    else
    {
        channel->leave(sd->channels[k], sd);
    }
}

void map_parse_pet_say(int fd)
{
    char message[500];

    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd || !sd->pd)
        return;

    const int len = RFIFOW(fd, 2);
    if (len > 500 || len < 6)
        return;
    safestrncpy(message, (char*)RFIFOP(fd, 4), len - 4);
    send_slave_say(sd, &sd->pd->bl, sd->pd->pet.name, message);
}

void map_parse_pet_emote(int fd)
{
    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd || !sd->pd)
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
    if (!sd || !sd->pd)
        return;
    short x = RFIFOW(fd, 6);
    short y = RFIFOW(fd, 8);

    struct block_list *pdBl = &sd->pd->bl;
    if (map->getcell(pdBl->m, x, y, CELL_CHKPASS))
        unit->walktoxy(pdBl, x, y, 0);
}

void map_parse_pet_dir(int fd)
{
    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd || !sd->pd)
        return;
    unit->setdir(&sd->pd->bl, RFIFOB(fd, 8));
}

void map_parse_homun_say(int fd)
{
    char message[500];

    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd)
        return;
    const int len = RFIFOW(fd, 2);
    if (len > 500 || len < 6)
        return;
    safestrncpy(message, (char*)RFIFOP(fd, 4), len - 4);
    if (sd->md && sd->md->db)
        send_slave_say(sd, &sd->md->bl, sd->md->db->name, message);
    else if (sd->hd && homun_alive(sd->hd))
        send_slave_say(sd, &sd->hd->bl, sd->hd->homunculus.name, message);
}

void map_parse_homun_emote(int fd)
{
    TBL_PC* sd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!sd)
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
    if (!sd || !sd->pd)
        return;
    if (sd->md && sd->md->db)
        unit->setdir(&sd->md->bl, RFIFOB(fd, 8));
    else if (sd->hd && homun_alive(sd->hd))
        unit->setdir(&sd->hd->bl, RFIFOB(fd, 8));
}
