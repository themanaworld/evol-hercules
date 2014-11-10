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

#include "login/config.h"
#include "login/parse.h"

#include "../../../common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

HPExport struct hplugin_info pinfo =
{
    "evol_login",
    SERVER_TYPE_LOGIN,
    "0.1",
    HPM_VERSION
};

HPExport void plugin_init (void)
{
    session = GET_SYMBOL("session");
    sockt = GET_SYMBOL("sockt");

    login = GET_SYMBOL("login");

    addPacket(0x7530, 22, login_parse_version, hpParse_Login);
    addHookPre("login->parse_client_login", elogin_parse_client_login_pre);
}

HPExport void server_preinit (void)
{
    iMalloc = GET_SYMBOL("iMalloc");
    addLoginConf("update_server", config_update_server);
}

HPExport void server_online (void)
{
}

HPExport void plugin_final (void)
{
}
