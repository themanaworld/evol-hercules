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
#include "../../../map/pc.h"
#include "../../../map/script.h"

#include "map/dummy.h"
#include "map/parse.h"
#include "map/script.h"

#include "../../../common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

HPExport struct hplugin_info pinfo =
{
    "evol_map",
    SERVER_TYPE_MAP,
    "0.1",
    HPM_VERSION
};

/* run when server starts */
HPExport void plugin_init (void) {
    /* core interfaces */
    iMalloc = GET_SYMBOL("iMalloc");

    /* map-server interfaces */
    script = GET_SYMBOL("script");
    clif = GET_SYMBOL("clif");
    pc = GET_SYMBOL("pc");
    strlib = GET_SYMBOL("strlib");
    session = GET_SYMBOL("session");
    sockt = GET_SYMBOL("sockt");

    addScriptCommand("setcamnpc", "*", dummy);
    addScriptCommand("restorecam", "", dummy);
    addScriptCommand("npctalk3", "s", dummy);
    addScriptCommand("closedialog", "", dummy);
    addScriptCommand("shop", "s", dummy);
    addScriptCommand("getitemlink", "s", dummyStr);
    addScriptCommand("l", "s*", l);
    addScriptCommand("getlang", "", getLang);
    addScriptCommand("setlang", "i", dummy);
    addScriptCommand("requestlang", "*", dummy);
    addScriptCommand("getq", "i", dummyInt);
    addScriptCommand("setq", "ii", dummy);
    addScriptCommand("getnpcdir", "*", dummyInt);
    addScriptCommand("setnpcdir", "*", dummy);
    addScriptCommand("rif", "is*", dummyStr);
    addScriptCommand("countitemcolor", "*", dummyInt);
    addScriptCommand("getclientversion", "", getClientVersion);
    // must be replaced to misceffect
    addScriptCommand("misceffect2", "i*", dummy);

    addPacket(0x7530, 22, map_parse_version, hpClif_Parse);
}

HPExport void server_preinit (void) {
}

HPExport void server_online (void) {
}

HPExport void plugin_final (void) {
}
