// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../common/HPMi.h"
#include "../../../common/malloc.h"
#include "../../../common/mmo.h"
#include "../../../common/socket.h"
#include "../../../common/strlib.h"
#include "../../../map/channel.h"
#include "../../../map/clif.h"
#include "../../../map/pc.h"
#include "../../../map/pet.h"
#include "../../../map/unit.h"

#include "map/parse.h"
#include "map/send.h"
#include "map/map.h"
#include "map/data/session.h"
#include "map/struct/sessionext.h"

void map_parse_version(int fd)
{
    struct SessionExt *data = session_get(fd);
    data->clientVersion = RFIFOL(fd, 2);
}

void map_parse_join_channel(int fd)
{
    char name[24];
    char *p;
    struct map_session_data* sd = (struct map_session_data*)session[fd]->session_data;
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
    struct map_session_data* sd = (struct map_session_data*)session[fd]->session_data;
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

    struct map_session_data* sd = (struct map_session_data*)session[fd]->session_data;
    const int len = RFIFOW(fd, 2);
    if (len > 500 || len < 6)
        return;
    safestrncpy(message, (char*)RFIFOP(fd, 4), len - 4);
    send_pet_say(sd, message);
}

void map_parse_pet_emote(int fd)
{
    struct map_session_data* sd = (struct map_session_data*)session[fd]->session_data;
    if (!sd)
        return;
    const time_t t = time(NULL);
    if (sd->emotionlasttime + 1 >= t)
    { // not more than 1 per second
        sd->emotionlasttime = t;
        return;
    }

    sd->emotionlasttime = t;
    send_pet_emote(sd, RFIFOB(fd, 2));
}

void map_parse_set_status(int fd)
{
    struct SessionExt *data = session_get(fd);
    data->state = RFIFOB(fd, 2);
}

void map_parse_get_online_list(int fd)
{
    emap_online_list(fd);
}

void map_parse_pet_move(int fd)
{
    struct map_session_data* sd = (struct map_session_data*)session[fd]->session_data;
    if (!sd || !sd->pd)
        return;
    short x = RFIFOW(fd, 6);
    short y = RFIFOW(fd, 8);

    struct block_list *pdBl = &sd->pd->bl;
    if (map->getcell(pdBl->m, x, y, CELL_CHKPASS))
        unit->walktoxy(pdBl, x, y, 0);
}
