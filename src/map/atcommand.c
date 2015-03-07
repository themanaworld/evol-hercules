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
#include "../../../common/timer.h"
#include "../../../map/atcommand.h"
#include "../../../map/map.h"
#include "map/atcommand.h"
#include "map/lang.h"

const char* eatcommand_msgsd(struct map_session_data *sd, int *msgPtr)
{
    const int msg_number = *msgPtr;
    if (!(msg_number >= 0 && msg_number < MAX_MSG))
        return "??";
    return lang_pctrans(atcommand->msg_table[0][msg_number], sd);
}

const char* eatcommand_msgfd(int *fdPtr, int *msgPtr)
{
    const int msg_number = *msgPtr;
    const int fd = *fdPtr;
    struct map_session_data *sd = session_isValid(fd) ? session[fd]->session_data : NULL;
    if (!(msg_number >= 0 && msg_number < MAX_MSG))
        return "??";
    return lang_pctrans(atcommand->msg_table[0][msg_number], sd);
}
