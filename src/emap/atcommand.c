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
#include "map/atcommand.h"
#include "map/clif.h"
#include "map/map.h"
#include "map/pc.h"
#include "map/skill.h"

#include "plugins/HPMHooking.h"

#include "emap/atcommand.h"
#include "emap/lang.h"
#include "emap/inter.h"

const char* eatcommand_msgsd_pre(struct map_session_data **sdPtr,
                                 int *msgPtr)
{
    const int msg_number = *msgPtr;
    struct map_session_data *sd = *sdPtr;
    if (!(msg_number >= 0 && msg_number < MAX_MSG))
    {
        hookStop();
        return "??";
    }
    if (*msgPtr == 1435)
    {
        hookStop();
        // service message, must be not translated
        return "You're now in the '#%s' channel for '%s'";
    }
    else if (*msgPtr == 1403)
    {
        hookStop();
        // service message, must be not translated
        return "You're now in the '#%s' channel for '-'";
    }
    hookStop();
    return lang_pctrans(atcommand->msg_table[0][msg_number], sd);
}

const char* eatcommand_msgfd_pre(int *fdPtr,
                                 int *msgPtr)
{
    const int msg_number = *msgPtr;
    const int fd = *fdPtr;
    struct map_session_data *sd = sockt->session_is_valid(fd) ? sockt->session[fd]->session_data : NULL;
    if (!(msg_number >= 0 && msg_number < MAX_MSG))
    {
        hookStop();
        return "??";
    }
    hookStop();
    return lang_pctrans(atcommand->msg_table[0][msg_number], sd);
}

ACMD2(setSkill)
{
    int skill_id = 0;
    int skill_level = 0;

    if (!*message || sscanf(message, "%5d %2d", &skill_id, &skill_level) < 2)
    {
        char buf[100];

        if (!*message ||
            sscanf(message, "%99s %2d", &buf[0], &skill_level) != 2 ||
            !script->get_constant(buf, &skill_id))
        {
            const char* text = info->help;
            if (text)
                clif->messageln (fd, text);
            return false;
        }
    }
    if (!skill->get_index(skill_id))
    {
        clif->message(fd, msg_fd(fd,198)); // This skill number doesn't exist.
        return false;
    }

    pc->skill(sd, skill_id, skill_level, 0);
    clif->message(fd, msg_fd(fd,70)); // You have learned the skill.

    return true;
}

ACMD2(slide)
{
    int x = 0;
    int y = 0;
    if (!*message || sscanf(message, "%4d %4d", &x, &y) < 2)
    {
        const char* text = info->help;
        if (text)
            clif->messageln (fd, text);
        return false;
    }

    if (!sd)
    {
        clif->message(fd, msg_fd(fd, 3)); // Character not found.
        return false;
    }

    const int m = sd->bl.m;
    if (x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys)
    {
        ShowError("slide: attempt to place player %s (%d:%d) on invalid coordinates (%d,%d)\n", sd->status.name, sd->status.account_id, sd->status.char_id, x, y);
        return false;
    }
    if (map->getcell(m, &sd->bl, x, y, CELL_CHKNOPASS) && pc_get_group_level(sd) < battle->bc->gm_ignore_warpable_area)
    {
        clif->message(fd, msg_fd(fd, 2));
        return false;
    }
    clif->slide(&sd->bl, x, y);
    unit->movepos(&sd->bl, x, y, 1, 0);
    return true;
}

ACMD1(mapExit)
{
    int code = 1;
    if (!*message || sscanf(message, "%5d", &code) < 1)
        code = 1;

    map->retval = code;
    map->do_shutdown();
    return true;
}

// 100 - terminate all servers
// 101 - restart all servers
// 102 - restart char and map servers
// 103 - restart map server
// 104 - git pull and restart all servers
// 105 - build all
// 106 - rebuild all
ACMD1(serverExit)
{
    int code = 0;
    if (!*message || sscanf(message, "%5d", &code) < 1)
        return false;

    send_char_exit(code);

    map->retval = code;
    map->do_shutdown();

    return true;
}
