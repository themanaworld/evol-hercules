// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/atcommand.h"
#include "map/clif.h"
#include "map/map.h"
#include "map/pc.h"
#include "map/skill.h"
#include "emap/atcommand.h"
#include "emap/lang.h"

const char* eatcommand_msgsd(struct map_session_data *sd, int *msgPtr)
{
    const int msg_number = *msgPtr;
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

const char* eatcommand_msgfd(int *fdPtr, int *msgPtr)
{
    const int msg_number = *msgPtr;
    const int fd = *fdPtr;
    struct map_session_data *sd = session_isValid(fd) ? session[fd]->session_data : NULL;
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

    if (!message || !*message || sscanf(message, "%5d %2d", &skill_id, &skill_level) < 2)
    {
        const char* text = info->help;

        if (text)
            clif->messageln (fd, text);

        return false;
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
