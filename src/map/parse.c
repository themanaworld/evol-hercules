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

#include "map/parse.h"
#include "map/send.h"
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
