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
#include "../../../login/login.h"

#include "login/parse.h"

#include "../../../common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

HPExport struct hplugin_info pinfo =
{
    "evol_login",
    SERVER_TYPE_LOGIN,
    "0.1",
    HPM_VERSION
};

/* run when server starts */
HPExport void plugin_init (void) {
    /* core interfaces */
    iMalloc = GET_SYMBOL("iMalloc");

    addPacket(0x7530, 22, login_parse_version, hpParse_Login);
}

HPExport void server_preinit (void) {
}

HPExport void server_online (void) {
}

HPExport void plugin_final (void) {
}
