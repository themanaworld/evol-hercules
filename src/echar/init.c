// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mapindex.h"
#include "common/mmo.h"
#include "common/packets.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/sql.h"
#include "common/timer.h"
#include "char/char.h"
#include "char/geoip.h"
#include "char/int_auction.h"
#include "char/int_elemental.h"
#include "char/int_guild.h"
#include "char/int_homun.h"
#include "char/int_mail.h"
#include "char/int_mercenary.h"
#include "char/int_party.h"
#include "char/int_pet.h"
#include "char/int_quest.h"
#include "char/int_rodex.h"
#include "char/int_storage.h"
#include "char/inter.h"
#include "char/loginif.h"
#include "char/mapif.h"

#include "ecommon/config.h"
#include "ecommon/init.h"
#include "echar/char.h"
#include "echar/config.h"

#include "plugins/HPMHooking.h"

HPExport struct HPMHooking_interface HPMHooking_s;

#include "common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

HPExport struct hplugin_info pinfo =
{
    "evol_char",
    SERVER_TYPE_CHAR,
    "0.1",
    HPM_VERSION
};

HPExport void plugin_init (void)
{
    addPacket(0x0061, 50, echar_parse_change_paassword, hpParse_Char);
    addPacket(0x5001, 7, echar_parse_login_password_change_ack, hpParse_FromLogin);
    addPacket(0x5002, 4, echar_parse_map_serverexit, hpParse_FromMap);

    packets->addLen(0xb17 + evolPacketOffset, 19);
    packets->addLen(0x970, 36);

    addHookPre(chr, parse_char_create_new_char, echar_parse_char_create_new_char);
    addHookPre(chr, creation_failed, echar_creation_failed);
    addHookPre(chr, parse_char_connect, echar_parse_char_connect_pre);
    addHookPre(chr, send_HC_ACK_CHARINFO_PER_PAGE_tail, echar_send_HC_ACK_CHARINFO_PER_PAGE_tail_pre);

    // gender stuff:
    addHookPre(chr, mmo_gender, echar_mmo_gender);
    addHookPre(chr, changecharsex, echar_changecharsex);

    addHookPost(chr, send_HC_ACK_CHARINFO_PER_PAGE, echar_send_HC_ACK_CHARINFO_PER_PAGE_post);
    addHookPost(chr, mmo_char_send_characters, echar_mmo_char_send_characters_post);
    addHookPost(chr, parse_char_connect, echar_parse_char_connect_post);
}

HPExport void server_preinit (void)
{
    interfaces_init_common();

    setDefaultMap();
    addMapInterConf("default_map", config_default_map);
    addMapInterConf("default_x", config_default_x);
    addMapInterConf("default_y", config_default_y);

    addCharConf("min_char_class", config_min_char_class);
    addCharConf("max_char_class", config_max_char_class);
    addCharConf("min_cloth_color", config_min_look);
    addCharConf("max_cloth_color", config_max_look);
}

HPExport void server_online (void)
{
    common_online();
}

HPExport void plugin_final (void)
{
    commonClean();
}
