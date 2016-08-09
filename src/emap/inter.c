// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/chrif.h"

void send_char_exit(const int code)
{
    WFIFOHEAD(chrif->fd, 4);
    WFIFOW(chrif->fd, 0) = 0x5002;
    WFIFOW(chrif->fd, 2) = code;
    WFIFOSET(chrif->fd, 4);
}
