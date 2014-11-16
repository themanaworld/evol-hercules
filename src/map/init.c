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
#include "../../../map/clif.h"
#include "../../../map/itemdb.h"
#include "../../../map/npc.h"
#include "../../../map/pc.h"
#include "../../../map/script.h"

#include "map/dummy.h"
#include "map/parse.h"
#include "map/script.h"
#include "map/pc.h"

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
    iMalloc = GET_SYMBOL("iMalloc");

    script = GET_SYMBOL("script");
    clif = GET_SYMBOL("clif");
    pc = GET_SYMBOL("pc");
    npc = GET_SYMBOL("npc");
    strlib = GET_SYMBOL("strlib");
    session = GET_SYMBOL("session");
    sockt = GET_SYMBOL("sockt");
    itemdb = GET_SYMBOL("itemdb");

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
    addScriptCommand("getq", "i", dummyInt);
    addScriptCommand("setq", "ii", dummy);
    addScriptCommand("getnpcdir", "*", dummyInt);
    addScriptCommand("setnpcdir", "*", dummy);
    addScriptCommand("rif", "is*", dummyStr);
    addScriptCommand("countitemcolor", "*", dummyInt);
    addScriptCommandDeprecated("getclientversion", "", getClientVersion);
    // must be replaced to misceffect
    addScriptCommand("misceffect2", "i*", dummy);

    addPacket(0x7530, 22, map_parse_version, hpClif_Parse);
    addHookPre("pc->readparam", epc_readparam_pre);
    addHookPre("pc->setregistry", epc_setregistry);

    langScriptId = script->add_str("Lang");
}

HPExport void server_preinit (void)
{
}

HPExport void server_online (void)
{
}

HPExport void plugin_final (void)
{
}
