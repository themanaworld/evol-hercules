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
#include "../../../map/channel.h"
#include "../../../map/chat.h"
#include "../../../map/chrif.h"
#include "../../../map/clif.h"
#include "../../../map/duel.h"
#include "../../../map/elemental.h"
#include "../../../map/homunculus.h"
#include "../../../map/instance.h"
#include "../../../map/intif.h"
#include "../../../map/irc-bot.h"
#include "../../../map/itemdb.h"
#include "../../../map/mail.h"
#include "../../../map/mapreg.h"
#include "../../../map/mercenary.h"
#include "../../../map/mob.h"
#include "../../../map/npc.h"
#include "../../../map/party.h"
#include "../../../map/pet.h"
#include "../../../map/pc.h"
#include "../../../map/script.h"
#include "../../../map/storage.h"
#include "../../../map/trade.h"
#include "../../../map/quest.h"

#include "common/config.h"
#include "common/init.h"
#include "map/clif.h"
#include "map/itemdb.h"
#include "map/lang.h"
#include "map/map.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/unit.h"
#include "map/parse.h"
#include "map/permission.h"
#include "map/pc.h"
#include "map/quest.h"
#include "map/script.h"
#include "map/skill.h"
#include "map/status.h"

#include "../../../common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

extern int langScriptId;

HPExport struct hplugin_info pinfo =
{
    "evol_map",
    SERVER_TYPE_MAP,
    "0.1",
    HPM_VERSION
};

HPExport void plugin_init (void)
{
//    HPM_map_add_group_permission = GET_SYMBOL("addGroupPermission");

    addScriptCommand("setcamnpc", "*", setCamNpc);
    addScriptCommand("setcam", "ii", setCam);
    addScriptCommand("movecam", "ii", moveCam);
    addScriptCommand("restorecam", "", restoreCam);
    addScriptCommand("npctalk3", "s", npcTalk3);
    addScriptCommand("closedialog", "", closeDialog);
    addScriptCommand("shop", "s", shop);
    addScriptCommand("getitemlink", "s", getItemLink);
    addScriptCommand("l", "s*", l);
    addScriptCommand("lg", "s*", lg);
    addScriptCommand("requestlang", "v", requestLang);
    addScriptCommand("requestitem", "v", requestItem);
    addScriptCommand("requestitems", "v*", requestItems);
    addScriptCommand("getq", "i", getq);
    addScriptCommand("setq", "ii", setq);
    addScriptCommand("setnpcdir", "*", setNpcDir);
    addScriptCommand("rif", "is*", rif);
    addScriptCommand("countitemcolor", "v*", countItemColor);
    addScriptCommand("misceffect", "i*", miscEffect);
    addScriptCommand("setmapmask", "si", setMapMask);
    addScriptCommand("addmapmask", "si", addMapMask);
    addScriptCommand("removemapmask", "si", removeMapMask);
    addScriptCommand("getmapmask", "s", getMapMask);
    addScriptCommand("setnpcsex", "*", setNpcSex);
    addScriptCommand("showavatar", "*", showAvatar);
    addScriptCommand("setavatardir", "i", setAvatarDir);
    addScriptCommand("setavataraction", "i", setAvatarAction);
    addScriptCommand("clear", "", clear);
    addScriptCommand("changemusic", "ss", changeMusic);
    addScriptCommand("setnpcdialogtitle", "s", setNpcDialogTitle);
    addScriptCommand("getmapname", "", getMapName);
    addScriptCommand("unequipbyid", "i", unequipById);
    addScriptCommand("ispcdead", "", isPcDead);
    addScriptCommand("areatimer", "siiiii*", areaTimer);
    addScriptCommand("getareadropitem", "siiiiv*", getAreaDropItem);

    do_init_langs();

    addPacket(0x7530, 22, map_parse_version, hpClif_Parse);
    addPacket(0xb07, 26, map_parse_join_channel, hpClif_Parse);
    addPacket(0xb09, 26, map_parse_part_channel, hpClif_Parse);
    addPacket(0xb0c, -1, map_parse_pet_say, hpClif_Parse);
    addPacket(0xb0d, 3, map_parse_pet_emote, hpClif_Parse);
    addPacket(0xb0e, 4, map_parse_set_status, hpClif_Parse);
    addPacket(0xb0f, 2, map_parse_get_online_list, hpClif_Parse);
    addPacket(0xb11, 10, map_parse_pet_move, hpClif_Parse);
    addPacket(0xb12, 9, map_parse_pet_dir, hpClif_Parse);
    addPacket(0xb13, -1, map_parse_homun_say, hpClif_Parse);
    addPacket(0xb14, 3, map_parse_homun_emote, hpClif_Parse);
    addPacket(0xb15, 9, map_parse_homun_dir, hpClif_Parse);

    addHookPre("pc->readparam", epc_readparam_pre);
    addHookPre("pc->setregistry", epc_setregistry);
    addHookPre("pc->equipitem_pos", epc_equipitem_pos);
    addHookPre("pc->unequipitem_pos", epc_unequipitem_pos);
    addHookPre("pc->can_attack", epc_can_attack);
    addHookPre("pc->takeitem", epc_takeitem);
    addHookPre("pc->validate_levels", epc_validate_levels);
    addHookPre("mob->deleteslave_sub", emob_deleteslave_sub);
    addHookPre("npc->parse_unknown_mapflag", enpc_parse_unknown_mapflag);
    addHookPre("npc->buysellsel", enpc_buysellsel);
    addHookPre("npc->db_checkid", enpc_db_checkid);
    addHookPre("clif->quest_send_list", eclif_quest_send_list);
    addHookPre("clif->quest_add", eclif_quest_add);
    addHookPre("clif->charnameack", eclif_charnameack);
    addHookPre("clif->sendlook", eclif_sendlook);
    addHookPre("clif->send", eclif_send);
    addHookPre("clif->set_unit_idle", eclif_set_unit_idle);
    addHookPre("clif->send_actual", eclif_send_actual);
    addHookPre("itemdb->is_item_usable", eitemdb_is_item_usable);
    addHookPre("itemdb->readdb_additional_fields", eitemdb_readdb_additional_fields);
    addHookPre("unit->can_move", eunit_can_move);
    addHookPre("unit->walktoxy", eunit_walktoxy);

    addHookPost("clif->getareachar_unit", eclif_getareachar_unit_post);
    addHookPost("clif->authok", eclif_authok_post);
    addHookPost("clif->changemap", eclif_changemap_post);
    addHookPost("clif->set_unit_idle", eclif_set_unit_idle_post);
    addHookPost("status->set_viewdata", estatus_set_viewdata_post);
    addHookPost("clif->set_unit_walking", eclif_set_unit_walking);
    addHookPost("clif->move", eclif_move);
    addHookPost("map->addflooritem", emap_addflooritem_post);
    addHookPost("skill->check_condition_castend", eskill_check_condition_castend_post);

    langScriptId = script->add_str("Lang");
}

HPExport void server_preinit (void)
{
    interfaces_init_common();

    atcommand = GET_SYMBOL("atcommand");
    battle = GET_SYMBOL("battle");
    bg = GET_SYMBOL("battlegrounds");
    buyingstore = GET_SYMBOL("buyingstore");
    clif = GET_SYMBOL("clif");
    chrif = GET_SYMBOL("chrif");
    guild = GET_SYMBOL("guild");
    gstorage = GET_SYMBOL("gstorage");
    homun = GET_SYMBOL("homun");
    map = GET_SYMBOL("map");
    ircbot = GET_SYMBOL("ircbot");
    itemdb = GET_SYMBOL("itemdb");
    logs = GET_SYMBOL("logs");
    mail = GET_SYMBOL("mail");
    instance = GET_SYMBOL("instance");
    script = GET_SYMBOL("script");
    searchstore = GET_SYMBOL("searchstore");
    skill = GET_SYMBOL("skill");
    vending = GET_SYMBOL("vending");
    pc = GET_SYMBOL("pc");
    pcg = GET_SYMBOL("pc_groups");
    party = GET_SYMBOL("party");
    storage = GET_SYMBOL("storage");
    trade = GET_SYMBOL("trade");
    status = GET_SYMBOL("status");
    chat = GET_SYMBOL("chat");
    duel = GET_SYMBOL("duel");
    elemental = GET_SYMBOL("elemental");
    intif = GET_SYMBOL("intif");
    mercenary = GET_SYMBOL("mercenary");
    mob = GET_SYMBOL("mob");
    unit = GET_SYMBOL("unit");
    npc = GET_SYMBOL("npc");
    mapreg = GET_SYMBOL("mapreg");
    pet = GET_SYMBOL("pet");
    path = GET_SYMBOL("path");
    quest = GET_SYMBOL("quest");
    npc_chat = GET_SYMBOL("npc_chat");
    libpcre = GET_SYMBOL("libpcre");
    mapit = GET_SYMBOL("mapit");
    mapindex = GET_SYMBOL("mapindex");
    channel = GET_SYMBOL("channel");

    setDefaultMap();
    addMapInterConf("default_map", config_default_map);
    addMapInterConf("default_x", config_default_x);
    addMapInterConf("default_y", config_default_y);

    addHookPre("quest->read_db_sub", equest_read_db_sub);
    addGroupPermission("send_gm", permission_send_gm_flag);
    addGroupPermission("show_client_version", permission_show_client_version_flag);
}

HPExport void server_online (void)
{
}

HPExport void plugin_final (void)
{
    do_final_langs();
    commonClean();
}
