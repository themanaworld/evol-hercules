// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mapindex.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/channel.h"
#include "map/chat.h"
#include "map/chrif.h"
#include "map/clif.h"
#include "map/duel.h"
#include "map/elemental.h"
#include "map/homunculus.h"
#include "map/guild.h"
#include "map/instance.h"
#include "map/intif.h"
#include "map/irc-bot.h"
#include "map/itemdb.h"
#include "map/mail.h"
#include "map/mapreg.h"
#include "map/mercenary.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/party.h"
#include "map/pet.h"
#include "map/pc.h"
#include "map/script.h"
#include "map/storage.h"
#include "map/trade.h"
#include "map/quest.h"

#include "ecommon/config.h"
#include "ecommon/init.h"
#include "emap/atcommand.h"
#include "emap/clif.h"
#include "emap/itemdb.h"
#include "emap/lang.h"
#include "emap/mail.h"
#include "emap/map.h"
#include "emap/mob.h"
#include "emap/npc.h"
#include "emap/unit.h"
#include "emap/parse.h"
#include "emap/permission.h"
#include "emap/pc.h"
#include "emap/quest.h"
#include "emap/script.h"
#include "emap/skill.h"
#include "emap/status.h"

#include "common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

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
    status_init();

    addAtcommand("setskill", setSkill);

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
    addScriptCommand("setmount", "?", setMount);
    addScriptCommand("clientcommand", "s", clientCommand);
    addScriptCommand("isunitwalking", "?", isUnitWalking);

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

    addHookPre("atcommand->msgfd", eatcommand_msgfd);
    addHookPre("atcommand->msgsd", eatcommand_msgsd);
    addHookPre("pc->readparam", epc_readparam_pre);
    addHookPre("pc->setregistry", epc_setregistry);
    addHookPre("pc->equipitem_pos", epc_equipitem_pos);
    addHookPre("pc->unequipitem_pos", epc_unequipitem_pos);
    addHookPre("pc->can_attack", epc_can_attack);
    addHookPre("pc->takeitem", epc_takeitem);
    addHookPre("pc->validate_levels", epc_validate_levels);
    addHookPre("pc->check_job_name", epc_check_job_name);
    addHookPre("pc->delitem", epc_delitem_pre);
    addHookPre("mob->deleteslave_sub", emob_deleteslave_sub);
    addHookPre("npc->parse_unknown_mapflag", enpc_parse_unknown_mapflag);
    addHookPre("npc->buysellsel", enpc_buysellsel);
    addHookPre("npc->db_checkid", enpc_db_checkid);
    addHookPre("clif->quest_send_list", eclif_quest_send_list);
    addHookPre("clif->quest_add", eclif_quest_add);
    addHookPre("clif->charnameack", eclif_charnameack);
    addHookPre("clif->getareachar_item", eclif_getareachar_item);
    addHookPre("clif->dropflooritem", eclif_dropflooritem);
    addHookPre("clif->sendlook", eclif_sendlook);
    addHookPre("clif->send", eclif_send);
    addHookPre("clif->set_unit_idle", eclif_set_unit_idle);
    addHookPre("clif->send_actual", eclif_send_actual);
    addHookPre("clif->pLoadEndAck", eclif_parse_LoadEndAck_pre);
    addHookPre("itemdb->is_item_usable", eitemdb_is_item_usable);
    addHookPre("itemdb->readdb_additional_fields", eitemdb_readdb_additional_fields);
    addHookPre("unit->can_move", eunit_can_move);
    addHookPre("unit->walktoxy", eunit_walktoxy);
    addHookPre("mail->invalid_operation", email_invalid_operation);
    addHookPre("status->calc_pc_additional", estatus_calc_pc_additional);

    addHookPost("clif->getareachar_unit", eclif_getareachar_unit_post);
    addHookPost("clif->authok", eclif_authok_post);
    addHookPost("clif->changemap", eclif_changemap_post);
    addHookPost("clif->set_unit_idle", eclif_set_unit_idle_post);
    addHookPost("status->set_viewdata", estatus_set_viewdata_post);
    addHookPost("status->read_job_db_sub", estatus_read_job_db_sub);
    addHookPost("status->calc_pc_", estatus_calc_pc_);
    addHookPost("clif->set_unit_walking", eclif_set_unit_walking);
    addHookPost("clif->move", eclif_move);
    addHookPost("map->addflooritem", emap_addflooritem_post);
    addHookPost("skill->check_condition_castend", eskill_check_condition_castend_post);
    addHookPost("pc->additem", epc_additem_post);
    addHookPost("pc->isequip", epc_isuseequip_post);
    addHookPost("pc->isUseitem", epc_isuseequip_post);
    addHookPost("pc->useitem", epc_useitem_post);
    addHookPost("pc->equipitem", epc_equipitem_post);
    addHookPost("pc->unequipitem", epc_unequipitem_post);
    addHookPost("pc->setnewpc", epc_setnewpc);
    addHookPost("pc->delitem", epc_delitem_post);

    langScriptId = script->add_str("Lang");
}

HPExport void server_preinit (void)
{
    interfaces_init_common();

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
