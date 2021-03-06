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
#include "map/map.h"
#include "map/npc.h"
#include "map/status.h"

unsigned int permission_send_gm_flag = UINT_MAX - 1;
unsigned int permission_show_client_version_flag = UINT_MAX - 2;
