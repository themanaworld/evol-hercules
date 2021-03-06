// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/packets.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/mapindex.h"
#include "login/lclif.h"
#include "login/lclif.p.h"
#include "login/login.h"

#include "ecommon/init.h"
#include "elogin/config.h"
#include "elogin/login.h"
#include "elogin/parse.h"

#include "plugins/HPMHooking.h"

HPExport struct HPMHooking_interface HPMHooking_s;

#include "common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

HPExport struct hplugin_info pinfo =
{
    "evol_login",
    SERVER_TYPE_LOGIN,
    "0.1",
    HPM_VERSION
};

HPExport void plugin_init (void)
{
    interfaces_init_common();

    addPacket(0x7530, 22, login_parse_version, hpParse_Login);
    addPacket(0x027c, 91, elogin_parse_client_login2, hpParse_Login);
    addPacket(0x5000, 54, elogin_parse_change_paassword, hpParse_FromChar);
    addPacket(0x5003, 4, elogin_parse_serverexit, hpParse_FromChar);
    packets->addLen(0x7531, -1);
    packets->addLen(0x63, -1);

    addHookPre(login, client_login, elogin_client_login_pre);
    addHookPre(login, check_password, elogin_check_password_pre);
    addHookPrePriv(lclif, parse_CA_CONNECT_INFO_CHANGED, elogin_parse_ping_pre);

    addHookPost(login, client_login, elogin_client_login_post);
}

HPExport void server_preinit (void)
{
    addLoginConf("update_server", config_update_server);
}

HPExport void server_online (void)
{
    common_online();
}

HPExport void plugin_final (void)
{
    config_final();
}
