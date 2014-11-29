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
#include "../../../char/char.h"
#include "../../../char/geoip.h"
#include "../../../char/int_auction.h"
#include "../../../char/int_elemental.h"
#include "../../../char/int_guild.h"
#include "../../../char/int_homun.h"
#include "../../../char/int_mail.h"
#include "../../../char/int_mercenary.h"
#include "../../../char/int_party.h"
#include "../../../char/int_pet.h"
#include "../../../char/int_quest.h"
#include "../../../char/int_storage.h"
#include "../../../char/inter.h"
#include "../../../char/loginif.h"
#include "../../../char/mapif.h"

#include "common/interfaces.h"
#include "char/char.h"
#include "char/config.h"

#include "../../../common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

HPExport struct hplugin_info pinfo =
{
    "evol_char",
    SERVER_TYPE_CHAR,
    "0.1",
    HPM_VERSION
};

HPExport void plugin_init (void)
{
    chr = GET_SYMBOL("chr");
    geoip = GET_SYMBOL("geoip");
    inter_auction = GET_SYMBOL("inter_auction");
    inter_elemental = GET_SYMBOL("inter_elemental");
    inter_guild = GET_SYMBOL("inter_guild");
    inter_homunculus = GET_SYMBOL("inter_homunculus");
    inter_mail = GET_SYMBOL("inter_mail");
    inter_mercenary = GET_SYMBOL("inter_mercenary");
    inter_party = GET_SYMBOL("inter_party");
    inter_pet = GET_SYMBOL("inter_pet");
    inter_quest = GET_SYMBOL("inter_quest");
    inter_storage = GET_SYMBOL("inter_storage");
    inter = GET_SYMBOL("inter");
    loginif = GET_SYMBOL("loginif");
    mapif = GET_SYMBOL("mapif");

    addHookPre("chr->parse_char_login_map_server", echar_parse_char_login_map_server);
}

HPExport void server_preinit (void)
{
    interfaces_init_common();

    addCharInterConf("inter_server_ip", config_inter_server_ip);
}

HPExport void server_online (void)
{
}

HPExport void plugin_final (void)
{
}
