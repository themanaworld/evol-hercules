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
#include "../../../map/pc.h"
#include "../../../map/quest.h"

#include "map/clif.h"

void eclif_quest_send_list(struct map_session_data *sd)
{
    hookStop();
    int fd = sd->fd;
    int i;
    int info_len = 15;
    int len = sd->avail_quests * info_len + 8;
    WFIFOHEAD(fd,len);
    WFIFOW(fd, 0) = 0x97a;
    WFIFOW(fd, 2) = len;
    WFIFOL(fd, 4) = sd->avail_quests;

    for (i = 0; i < sd->avail_quests; i++ )
    {
        struct quest_db *qi = quest->db(sd->quest_log[i].quest_id);
        WFIFOL(fd, i * info_len + 8) = sd->quest_log[i].quest_id;
        WFIFOB(fd, i * info_len + 12) = sd->quest_log[i].count[0]; // was state
        WFIFOL(fd, i * info_len + 13) = sd->quest_log[i].time - qi->time;
        WFIFOL(fd, i * info_len + 17) = sd->quest_log[i].time;
        WFIFOW(fd, i * info_len + 21) = 0;
    }

    WFIFOSET(fd, len);
}

void eclif_quest_add(struct map_session_data *sd, struct quest *qd)
{
    hookStop();
    int fd = sd->fd;
    int i;
    struct mob_db *monster;
    struct quest_db *qi = quest->db(qd->quest_id);

    WFIFOHEAD(fd, packet_len(0x2b3));
    WFIFOW(fd, 0) = 0x2b3;
    WFIFOL(fd, 2) = qd->quest_id;
    WFIFOB(fd, 6) = qd->count[0]; // was state;
    WFIFOB(fd, 7) = qd->time - qi->time;
    WFIFOL(fd, 11) = qd->time;
    WFIFOW(fd, 15) = 0;

    WFIFOSET(fd, 107);
}
