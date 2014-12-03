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
#include "../../../map/clif.h"
#include "../../../map/pc.h"

#include "map/parse.h"
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
    struct hChSysCh *channel = NULL;
    struct map_session_data* sd = (struct map_session_data*)session[fd]->session_data;
    int res = 0;
    if (!sd)
        return;

    safestrncpy(name, RFIFOP(fd, 2), 24);
    if (name[0] == '#')
        p = name + 1;
    else
        p = name;

    if (clif->hChSys->local && strcmpi(p, clif->hChSys->local_name) == 0)
    {
        if (!map->list[sd->bl.m].channel)
            clif->chsys_mjoin(sd);
        channel = map->list[sd->bl.m].channel;
        res = 1;
    }
    else if (clif->hChSys->ally && sd->status.guild_id && strcmpi(p, clif->hChSys->ally_name) == 0)
    {
        struct guild *g = sd->guild;
        if (g)
            channel = g->channel;
    }
    if (!res && (channel || (channel = strdb_get(clif->channel_db,p ))))
    {
        int k;
        for (k = 0; k < sd->channel_count; k++)
        {
            if (sd->channels[k] == channel)
                break;
        }
        if (k < sd->channel_count)
        {
            res = 2;
        }
        else if (channel->pass[0] == '\0' && !(channel->banned && idb_exists(channel->banned, sd->status.account_id)))
        {
            if (channel->type == hChSys_ALLY)
            {
                struct guild *g = sd->guild, *sg = NULL;
                for (k = 0; k < MAX_GUILDALLIANCE; k++)
                {
                    if (g->alliance[k].opposition == 0 && g->alliance[k].guild_id && (sg = guild->search(g->alliance[k].guild_id)))
                    {
                        if (!(sg->channel->banned && idb_exists(sg->channel->banned, sd->status.account_id)))
                            clif->chsys_join(sg->channel,sd);
                    }
                }
            }
            clif->chsys_join(channel,sd);
            res = 1;
        }
        else
        {
            res = 0;
        }
    }

    send_join_ack(fd, name, res);
}

