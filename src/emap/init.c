// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/conf.h"
#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mapindex.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/packets.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/achievement.h"
#include "map/battle.h"
#include "map/channel.h"
#include "map/chat.h"
#include "map/chrif.h"
#include "map/clan.h"
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
#include "map/refine.h"
#include "map/script.h"
#include "map/storage.h"
#include "map/trade.h"
#include "map/quest.h"
#include "map/rodex.h"
#include "map/stylist.h"

#include "ecommon/config.h"
#include "ecommon/init.h"
#include "emap/atcommand.h"
#include "emap/battle.h"
#include "emap/battleground.h"
#include "emap/chrif.h"
#include "emap/clan.h"
#include "emap/clif.h"
#include "emap/config.h"
#include "emap/console.h"
#include "emap/craft.h"
#include "emap/craftconf.h"
#include "emap/hashtable.h"
#include "emap/homunculus.h"
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
#include "emap/script_buildins.h"
#include "emap/skill.h"
#include "emap/status.h"

#include "emap/data/skilld.h"

#include "plugins/HPMHooking.h"

HPExport struct HPMHooking_interface HPMHooking_s;

#include "common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

extern int langScriptId;
extern int mountScriptId;
bool isInit;
char global_npc_str[1001];

HPExport struct hplugin_info pinfo =
{
    "evol_map",
    SERVER_TYPE_MAP,
    "0.1",
    HPM_VERSION
};

HPExport void plugin_init (void)
{
    isInit = false;
    *global_npc_str = 0;
    htreg_init();
    skilld_init();

    addAtcommand("setskill", setSkill);
    addAtcommand("slide", slide);
    addAtcommand("mapexit", mapExit);
    addAtcommand("serverexit", serverExit);
    addAtcommand("hugo", hugo);
    addAtcommand("linus", linus);
    addAtcommand("tee", tee);
    addAtcommand("log", log);
    addAtcommand("getname", getName);

    addCPCommand("serverexit", serverExit);

    addScriptCommand("chatjoin", "i*", chatJoin);
    addScriptCommand("setcamnpc", "*", setCamNpc);
    addScriptCommand("setcam", "ii", setCam);
    addScriptCommand("movecam", "ii", moveCam);
    addScriptCommand("restorecam", "", restoreCam);
    addScriptCommand("npctalk3", "s", npcTalk3);
    addScriptCommand("closedialog", "", closeDialog);
    addScriptCommand("closeclientdialog", "", closeClientDialog);
    addScriptCommand("shop", "s", shop);
    addScriptCommand("getitemlink", "v*", getItemLink);
    addScriptCommand("getinvindexlink", "i", getInvIndexLink);
    addScriptCommand("l", "s*", l);
    addScriptCommand("lg", "s*", lg);
    addScriptCommand("requestlang", "", requestLang);
    addScriptCommand("requestitem", "", requestItem);
    addScriptCommand("requestitems", "*", requestItems);
    addScriptCommand("requestitemindex", "", requestItemIndex);
    addScriptCommand("requestitemsindex", "*", requestItemsIndex);
    addScriptCommand("requestcraft", "*", requestCraft);
    addScriptCommand("initcraft", "s", initCraft);
    addScriptCommand("dumpcraft", "i", dumpCraft);
    addScriptCommand("deletecraft", "i", deleteCraft);
    addScriptCommand("findcraftentry", "ii", findCraftEntry);
    addScriptCommand("usecraft", "i", useCraft);
    addScriptCommand("getcraftcode", "i", getCraftCode);
    addScriptCommand("getcraftslotid", "ii", getCraftSlotId);
    addScriptCommand("getcraftslotamount", "ii", getCraftSlotAmount);
    addScriptCommand("validatecraft", "i", validateCraft);
    addScriptCommand("getcraftrecipe", "iirr", getCraftRecipe);
    addScriptCommand("getq", "i", getq);
    addScriptCommand("getq1", "i", getq);
    addScriptCommand("getq2", "i", getq2);
    addScriptCommand("getq3", "i", getq3);
    addScriptCommand("getqtime", "i", getqTime);
    addScriptCommand("setq", "ii*", setq);
    addScriptCommand("setnpcdir", "*", setNpcDir);
    addScriptCommand("npcsit", "*", npcSit);
    addScriptCommand("npcstand", "*", npcStand);
    addScriptCommand("npcwalkto", "ii?", npcWalkTo);
    addScriptCommand("rif", "is*", rif);
    addScriptCommand("setmapmask", "si", setMapMask);
    addScriptCommand("sendmapmask", "i*", sendMapMask);
    addScriptCommand("addmapmask", "si", addMapMask);
    addScriptCommand("removemapmask", "si", removeMapMask);
    addScriptCommand("getmapmask", "s", getMapMask);
    addScriptCommand("setnpcsex", "si", setNpcSex);
    addScriptCommand("showavatar", "*", showAvatar);
    addScriptCommand("setavatardir", "i", setAvatarDir);
    addScriptCommand("setavataraction", "i", setAvatarAction);
    addScriptCommand("clear", "", clear);
    addScriptCommand("changemusic", "ss", changeMusic);
    addScriptCommand("changeplayermusic", "s", changePlayerMusic);
    addScriptCommand("setnpcdialogtitle", "s", setNpcDialogTitle);
    addScriptCommand("getmapname", "", getMapName);
    addScriptCommand("unequipbyid", "i", unequipById);
    addScriptCommand("ispcdead", "", isPcDead);
    addScriptCommand("getareadropitem", "siiiiv*", getAreaDropItem);
    addScriptCommand("clientcommand", "s", clientCommand);
    addScriptCommand("isunitwalking", "?", isUnitWalking);
    addScriptCommand("failedrefindex", "i", failedRefIndex);
    addScriptCommand("downrefindex", "ii", downRefIndex);
    addScriptCommand("successrefindex", "ii", successRefIndex);
    addScriptCommand("successremovecardsindex", "i", successRemoveCardsIndex);
    addScriptCommand("failedremovecardsindex", "ii", failedRemoveCardsIndex);
    addScriptCommand("getcardbyindex", "ii", getCardByIndex);
    addScriptCommand("removecardbyindex", "ii", removeCardByIndex);
    addScriptCommand("setbgteam", "ii", setBgTeam);
    addScriptCommand("checknpccell", "siii", checkNpcCell);
    addScriptCommand("setcells", "siiiiis", setCells);
    addScriptCommand("delcells", "s", delCells);
    addScriptCommand("setmount", "i", setMount);
    addScriptCommand("setskin", "s", setSkin);
    addScriptCommand("emotion", "i??", emotion);
    addScriptCommand("setlook", "ii", setLook);
    addScriptCommand("htnew", "", htNew);
    addScriptCommand("htget", "is?", htGet);
    addScriptCommand("htput", "isv", htPut);
    addScriptCommand("htclear", "i", htClear);
    addScriptCommand("htdelete", "i", htDelete);
    addScriptCommand("htsize", "i", htSize);
    addScriptCommand("htexists", "i", htExists);
    addScriptCommand("htiterator", "i", htIterator);
    addScriptCommand("htifirstkey", "i", htiFirstKey);
    addScriptCommand("htilastkey", "i", htiLastKey);
    addScriptCommand("htinextkey", "i", htiNextKey);
    addScriptCommand("htiprevkey", "i", htiPrevKey);
    addScriptCommand("hticheck", "i", htiCheck);
    addScriptCommand("htidelete", "i", htiDelete);
    addScriptCommand("setfakecells", "iii??", setFakeCells);
    addScriptCommand("getlabel", "l", getLabel);
    addScriptCommand("tolabel", "i", toLabel);
    addScriptCommand("input", "r??", input);
    addScriptCommand("slide", "ii", slide);
    addScriptCommand("getitemoptionidbyindex", "ii", getItemOptionIdByIndex);
    addScriptCommand("getitemoptionvaluebyindex", "ii", getItemOptionValueByIndex);
    addScriptCommand("getitemoptionparambyindex", "ii", getItemOptionParamByIndex);
    addScriptCommand("setitemoptionbyindex", "iii*", setItemOptionByIndex);
    addScriptCommand("isinstance", "i", isInstance);
    addScriptCommand("readbattleparam","ii",readBattleParam);
    addScriptCommand("instanceowner", "i", InstanceOwner);
    addScriptCommand("aggravate", "i", aggravate);
    addScriptCommand("getnpcsubtype", "?", getNpcSubtype);
    addScriptCommand("kick", "v?", kick);
    addScriptCommand("getskillname", "i", getskillname);

    do_init_langs();

    addPacket(0x7530, 22, map_parse_version, hpClif_Parse);
    addPacket(0xb07 + evolPacketOffset, 26, map_parse_join_channel, hpClif_Parse);
    addPacket(0xb09 + evolPacketOffset, 26, map_parse_part_channel, hpClif_Parse);
    addPacket(0xb0c + evolPacketOffset, -1, map_parse_pet_say, hpClif_Parse);
    addPacket(0xb0d + evolPacketOffset, 3, map_parse_pet_emote, hpClif_Parse);
    addPacket(0xb0e + evolPacketOffset, 4, map_parse_set_status, hpClif_Parse);
    addPacket(0xb0f + evolPacketOffset, 2, map_parse_get_online_list, hpClif_Parse);
    addPacket(0xb11 + evolPacketOffset, 10, map_parse_pet_move, hpClif_Parse);
    addPacket(0xb12 + evolPacketOffset, 9, map_parse_pet_dir, hpClif_Parse);
    addPacket(0xb13 + evolPacketOffset, -1, map_parse_homun_say, hpClif_Parse);
    addPacket(0xb14 + evolPacketOffset, 3, map_parse_homun_emote, hpClif_Parse);
    addPacket(0xb15 + evolPacketOffset, 9, map_parse_homun_dir, hpClif_Parse);
    addPacket(0xb26 + evolPacketOffset, 6, map_clif_parse_useitem2, hpClif_Parse);
    packets->addLen(0xB00 + evolPacketOffset, 16);
    packets->addLen(0xB01 + evolPacketOffset, -1);
    packets->addLen(0xb02 + evolPacketOffset, 10);
    packets->addLen(0xb03 + evolPacketOffset, -1);
    packets->addLen(0xb04 + evolPacketOffset, -1);
    packets->addLen(0xb05 + evolPacketOffset, -1);
    packets->addLen(0xb06 + evolPacketOffset, -1);
    packets->addLen(0xb08 + evolPacketOffset, 27);
    packets->addLen(0xb0a + evolPacketOffset, -1);
    packets->addLen(0xb0b + evolPacketOffset, -1);
    packets->addLen(0xb10 + evolPacketOffset, -1);
    packets->addLen(0xb16 + evolPacketOffset, -1);
    packets->addLen(0xb17 + evolPacketOffset, 19);
    packets->addLen(0xb18 + evolPacketOffset, 28);
    packets->addLen(0xb19 + evolPacketOffset, 28);
    packets->addLen(0xb1a + evolPacketOffset, 34);
    packets->addLen(0xb1b + evolPacketOffset, 34);
    packets->addLen(0xb1c + evolPacketOffset, -1);
    packets->addLen(0xb1d + evolPacketOffset, 6);
    packets->addLen(0xb1e + evolPacketOffset, -1);
    packets->addLen(0xb1f + evolPacketOffset, -1);
    packets->addLen(0xb20 + evolPacketOffset, -1);
    packets->addLen(0xb21 + evolPacketOffset, 10);
    packets->addLen(0xb22 + evolPacketOffset, 10);
    packets->addLen(0xb23 + evolPacketOffset, -1);
    packets->addLen(0xb24 + evolPacketOffset, 23);
    packets->addLen(0xb25 + evolPacketOffset, 8);


    addHookPre(atcommand, msgfd, eatcommand_msgfd_pre);
    addHookPre(atcommand, msgsd, eatcommand_msgsd_pre);
    addHookPre(battle, weapon_attack, ebattle_weapon_attack_pre);
    addHookPre(bg, team_warp, ebg_team_warp_pre);
    addHookPre(pc, can_Adopt, epc_can_Adopt_pre);
    addHookPre(pc, additem, epc_additem_pre);
    addHookPre(pc, adoption, epc_adoption_pre);
    addHookPre(pc, readparam, epc_readparam_pre);
    addHookPre(pc, setparam, epc_setparam_pre);
    addHookPre(pc, setregistry, epc_setregistry_pre);
    addHookPre(pc, equipitem_pos, epc_equipitem_pos_pre);
    addHookPre(pc, unequipitem_pos, epc_unequipitem_pos_pre);
    addHookPre(pc, can_attack, epc_can_attack_pre);
    addHookPre(pc, takeitem, epc_takeitem_pre);
    addHookPre(pc, validate_levels, epc_validate_levels_pre);
    addHookPre(pc, check_job_name, epc_check_job_name_pre);
    addHookPre(pc, delitem, epc_delitem_pre);
    addHookPre(pc, dropitem, epc_dropitem_pre);
    addHookPre(pc, insert_card, epc_insert_card_pre);
    addHookPre(pc, process_chat_message, epc_process_chat_message_pre);
    addHookPre(pc, calc_skilltree_clear, epc_calc_skilltree_clear_pre);
    addHookPre(pc, calc_skilltree_bonus, epc_calc_skilltree_bonus_pre);
    addHookPre(pc, checkbaselevelup_sc, epc_checkbaselevelup_sc_pre);
    addHookPre(pc, resetskill_job, epc_resetskill_job_pre);
    addHookPre(pc, isDeathPenaltyJob, epc_isDeathPenaltyJob_pre);
    addHookPre(pc, read_skill_job_skip, epc_read_skill_job_skip_pre);
    addHookPre(mob, deleteslave_sub, emob_deleteslave_sub_pre);
    addHookPre(mob, read_db_additional_fields, emob_read_db_additional_fields_pre);
    addHookPre(mob, dead, emob_dead_pre);
    addHookPre(npc, parse_unknown_mapflag, enpc_parse_unknown_mapflag_pre);
    addHookPre(npc, buysellsel, enpc_buysellsel_pre);
    addHookPre(npc, db_checkid, enpc_db_checkid_pre);
    addHookPre(npc, duplicate_script_sub, enpc_duplicate_script_sub_pre);
    addHookPre(npc, unload, enpc_unload_pre);
    addHookPre(clif, quest_send_list, eclif_quest_send_list_pre);
    addHookPre(clif, quest_add, eclif_quest_add_pre);
    addHookPre(clif, blname_ack, eclif_blname_ack_pre);
    addHookPre(clif, homname_ack, eclif_homname_ack_pre);
    addHookPre(clif, mername_ack, eclif_mername_ack_pre);
    addHookPre(clif, petname_ack, eclif_petname_ack_pre);
    addHookPre(clif, elemname_ack, eclif_elemname_ack_pre);

    addHookPre(clif, charnameupdate, eclif_charnameupdate_pre);
    addHookPre(clif, getareachar_item, eclif_getareachar_item_pre);
    addHookPre(clif, dropflooritem, eclif_dropflooritem_pre);
    addHookPre(clif, disp_message, eclif_disp_message_pre);
    addHookPre(clif, sendlook, eclif_sendlook_pre);
    addHookPre(clif, send, eclif_send_pre);
    addHookPre(clif, sendbgemblem_area, eclif_sendbgemblem_area_pre);
    addHookPre(clif, sendbgemblem_single, eclif_sendbgemblem_single_pre);
    addHookPre(clif, set_unit_idle, eclif_set_unit_idle_pre);
    addHookPre(clif, send_actual, eclif_send_actual_pre);
    addHookPre(clif, pLoadEndAck, eclif_parse_LoadEndAck_pre);
    addHookPre(clif, skillinfoblock, eclif_skillinfoblock_pre);
    addHookPre(clif, addskill, eclif_addskill_pre);
    addHookPre(clif, skillinfo, eclif_skillinfo_pre);
    addHookPre(clif, rodex_icon, eclif_rodex_icon_pre);
    addHookPre(itemdb, is_item_usable, eitemdb_is_item_usable_pre);
    addHookPre(itemdb, readdb_additional_fields, eitemdb_readdb_additional_fields_pre);
    addHookPre(itemdb, destroy_item_data, edestroy_item_data_pre);
    addHookPre(unit, can_move, eunit_can_move_pre);
    addHookPre(unit, walk_toxy, eunit_walktoxy_pre);
    addHookPre(mail, invalid_operation, email_invalid_operation_pre);
    addHookPre(map, list_final, edo_final_maps_pre);
    addHookPre(map, cell2gat, emap_cell2gat_pre);
    addHookPre(map, gat2cell, emap_gat2cell_pre);
    addHookPre(map, getcellp, emap_getcellp_pre);
    addHookPre(map, setgatcell, emap_setgatcell_pre);
    addHookPre(map, iwall_set, emap_iwall_set_pre);
    addHookPre(map, iwall_get, emap_iwall_get_pre);
    addHookPre(map, iwall_remove, emap_iwall_remove_pre);
    addHookPre(script, get_val_npc_num, eget_val_npcscope_num_pre);
    addHookPre(script, get_val_ref_num, eget_val_npcscope_num_pre);
    addHookPre(script, get_val_npc_str, eget_val_npcscope_str_pre);
    addHookPre(script, get_val_ref_str, eget_val_npcscope_str_pre);
    addHookPre(script, set_reg_npc_num, eset_reg_npcscope_num_pre);
    addHookPre(script, set_reg_ref_num, eset_reg_npcscope_num_pre);
    addHookPre(script, set_reg_npc_str, eset_reg_npcscope_str_pre);
    addHookPre(script, set_reg_ref_str, eset_reg_npcscope_str_pre);
    addHookPre(script, reload, escript_reload_pre);
    addHookPre(script, load_translations, escript_load_translations_pre);
    addHookPre(script, load_parameters, escript_load_parameters_pre);
    addHookPre(script, hardcoded_constants, escript_hardcoded_constants_pre);
    addHookPre(script, run_use_script, escript_run_use_script_pre);
    addHookPre(status, calc_pc_additional, estatus_calc_pc_additional_pre);
    addHookPre(status, calc_pc_recover_hp, estatus_calc_pc_recover_hp_pre);
    addHookPre(homun, gainexp, ehomunculus_gainexp_pre);
    addHookPre(chrif, changesex, echrif_changesex);

    addHookPost(battle, calc_weapon_attack, ebattle_calc_weapon_attack_post);
    addHookPost(battle, calc_magic_attack, ebattle_calc_weapon_attack_post);
    addHookPost(battle, calc_misc_attack, ebattle_calc_weapon_attack_post);
    addHookPost(battle, check_arrows, ebattle_check_arrows_post);
    addHookPost(clan, join, eclan_join_post);
    addHookPost(clan, leave, eclan_leave_post);
    addHookPost(clif, addcards, eclif_addcards_post);
    addHookPost(clif, getareachar_unit, eclif_getareachar_unit_post);
    addHookPost(clif, authok, eclif_authok_post);
    addHookPost(clif, changemap, eclif_changemap_post);
    addHookPost(clif, set_unit_idle, eclif_set_unit_idle_post);
    addHookPost(clif, pLoadEndAck, eclif_parse_LoadEndAck_post);
    addHookPost(clif, spawn, eclif_spawn_post);
    addHookPost(clif, set_unit_walking, eclif_set_unit_walking_post);
    addHookPost(clif, move, eclif_move_post);
    addHookPost(clif, party_info, eclif_party_info_post);
    addHookPost(status, init, estatus_init_post);
    addHookPost(status, set_viewdata, estatus_set_viewdata_post);
    addHookPost(status, read_job_db_sub, estatus_read_job_db_sub_post);
    addHookPost(status, calc_pc_, estatus_calc_pc__post);
    addHookPost(status, calc_speed, estatus_calc_speed_post);
    addHookPost(status, calc_def, estatus_calc_def_post);
    addHookPost(status, calc_fix_aspd, estatus_calc_fix_aspd_post);
    addHookPost(status, change_start, estatus_change_start_post);
    addHookPost(map, addflooritem, emap_addflooritem_post);
    addHookPost(mob, read_db_mode_sub, emob_read_db_mode_sub_post);
    addHookPost(mob, spawn_dataset, emob_spawn_dataset_post);
    addHookPost(skill, check_condition_castend, eskill_check_condition_castend_post);
    addHookPost(skill, get_index, eskill_get_index_post);
    addHookPost(pc, additem, epc_additem_post);
    addHookPost(pc, isequip, epc_isequip_post);
    addHookPost(pc, isUseitem, epc_isequip_post);
    addHookPost(pc, useitem, epc_useitem_post);
    addHookPost(pc, equipitem, epc_equipitem_post);
    addHookPost(pc, unequipitem, epc_unequipitem_post);
    addHookPost(pc, setnewpc, epc_setnewpc_post);
    addHookPost(pc, dead, epc_dead_post);
    addHookPost(pc, delitem, epc_delitem_post);
    addHookPost(pc, dropitem, epc_dropitem_post);
    addHookPost(pc, takeitem, epc_takeitem_post);
    addHookPost(pc, can_insert_card_into, epc_can_insert_card_into_post);
    addHookPost(pc, insert_card, epc_insert_card_post);

    skill->castend_nodamage_id_unknown = eskill_castend_nodamage_id_unknown;
    skill->additional_effect_unknown = eskill_additional_effect_unknown;
    skill->counter_additional_effect_unknown = eskill_counter_additional_effect_unknown;
    skill->attack_combo2_unknown = eskill_attack_combo2_unknown;
    skill->attack_post_unknown = eskill_attack_post_unknown;
    skill->timerskill_notarget_unknown = eskill_timerskill_notarget_unknown;
    skill->unitsetting1_unknown = eskill_unitsetting1_unknown;
    skill->unit_onplace_unknown = eskill_unit_onplace_unknown;
    skill->check_condition_castend_unknown = eskill_check_condition_castend_unknown;
    skill->get_requirement_unknown = eskill_get_requirement_unknown;
    skill->castend_pos2_unknown = eskill_castend_pos2_unknown;
    skill->validate_additional_fields = eskill_validate_additional_fields;
    clif->useskill = eclif_useskill;
    clif->pWalkToXY = eclif_parse_WalkToXY;
    clif->pNpcStringInput = eclif_parse_NpcStringInput;
    pc->jobchange = epc_jobchange;
    itemdb->isidentified = eitemdb_isidentified;
    itemdb->isidentified2 = eitemdb_isidentified2;

    langScriptId = script->add_variable("Lang");
    mountScriptId = script->add_variable("mount");

    isInit = true;
}

HPExport void server_preinit (void)
{
    interfaces_init_common();
    htreg_defaults();

    setDefaultMap();
    addMapInterConf("default_map", config_default_map);
    addMapInterConf("default_x", config_default_x);
    addMapInterConf("default_y", config_default_y);
    addMapInterConf("warn_missing_translation", config_warn_missing_translation);

    addHookPre(quest, read_db_sub, equest_read_db_sub_pre);

    addGroupPermission("send_gm", permission_send_gm_flag);
    addGroupPermission("show_client_version", permission_show_client_version_flag);
}

HPExport void server_online (void)
{
    do_init_craft();
    do_init_craftconf();
    common_online();
}

HPExport void plugin_final (void)
{
    do_final_langs();
    do_final_craft();
    do_final_craftconf();
    commonClean();
    htreg_final();
    isInit = false;
}
