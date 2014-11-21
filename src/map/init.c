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

#include "common/interfaces.h"
#include "map/clif.h"
#include "map/dummy.h"
#include "map/lang.h"
#include "map/npc.h"
#include "map/parse.h"
#include "map/script.h"
#include "map/pc.h"
#include "map/quest.h"

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
    addScriptCommand("restorecam", "", restoreCam);
    addScriptCommand("npctalk3", "s", npcTalk3);
    addScriptCommand("closedialog", "", closeDialog);
    addScriptCommand("shop", "s", shop);
    addScriptCommand("getitemlink", "s", getItemLink);
    addScriptCommand("l", "s*", l);
    addScriptCommandDeprecated("getlang", "", getLang);
    addScriptCommandDeprecated("setlang", "i", setLang);
    addScriptCommand("requestlang", "v", requestLang);
    addScriptCommand("getq", "i", getq);
    addScriptCommand("setq", "ii", setq);
    addScriptCommand("getnpcdir", "*", dummyInt);
    addScriptCommand("setnpcdir", "*", dummy);
    addScriptCommand("rif", "is*", dummyStr);
    addScriptCommand("countitemcolor", "*", dummyInt);
    addScriptCommandDeprecated("getclientversion", "", getClientVersion);
    // must be replaced to misceffect
    addScriptCommand("misceffect2", "i*", dummy);

    do_init_langs();

    addPacket(0x7530, 22, map_parse_version, hpClif_Parse);
    addHookPre("pc->readparam", epc_readparam_pre);
    addHookPre("pc->setregistry", epc_setregistry);
    addHookPre("npc->checknear", enpc_checknear);
    addHookPre("clif->quest_send_list", eclif_quest_send_list);
    addHookPre("clif->quest_add", eclif_quest_add);
    addHookPre("clif->charnameack", eclif_charnameack);

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

    addHookPre("quest->read_db", equest_read_db);
}

HPExport void server_online (void)
{
}

HPExport void plugin_final (void)
{
    do_final_langs();
}
