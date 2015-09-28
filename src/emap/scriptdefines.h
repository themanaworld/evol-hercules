// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_SCRIPTDEFINES
#define EVOL_MAP_SCRIPTDEFINES

#define getSessionDataReturn(def) \
    if (!st->rid) \
    { \
        ShowWarning("!st->rid\n"); \
        script->reportsrc(st); \
        script_pushint(st, def); \
        return false; \
    } \
    TBL_PC *sd = script->rid2sd(st); \
    if (!sd) \
    { \
        ShowWarning("player not attached\n"); \
        script->reportsrc(st); \
        script_pushint(st, def); \
        return false; \
    } \
    struct SessionExt *data = session_get(sd->fd)

#define getSessionData(data) \
    if (!st->rid) \
    { \
        ShowWarning("!st->rid\n"); \
        script->reportsrc(st); \
        return false; \
    } \
    TBL_PC *sd = script->rid2sd(st); \
    if (!sd) \
    { \
        ShowWarning("player not attached\n"); \
        script->reportsrc(st); \
        return false; \
    } \
    struct SessionExt *data = session_get(sd->fd)

#define getMapData(m) \
        struct MapdExt *mapData = mapd_get(m); \
        if (!mapData) \
        { \
            ShowWarning("cant get map data\n"); \
            script->reportsrc(st); \
            return false; \
        }

#define getMapDataReturn(m, def) \
        struct MapdExt *mapData = mapd_get(m); \
        if (!mapData) \
        { \
            ShowWarning("cant get map data\n"); \
            script->reportsrc(st); \
            script_pushint(st, def); \
            return false; \
        }

#define getSD() \
    TBL_PC *sd = script->rid2sd(st); \
    if (!sd) \
    { \
        ShowWarning("player not attached\n"); \
        script->reportsrc(st); \
        return false; \
    }

#define getSDReturn(def) \
    TBL_PC *sd = script->rid2sd(st); \
    if (!sd) \
    { \
        ShowWarning("player not attached\n"); \
        script->reportsrc(st); \
        script_pushint(st, def); \
        return false; \
    }

#define getInventoryIndex(idx) \
    const int n = script_getnum(st, idx); \
    if (n < 0 || n >= MAX_INVENTORY) \
    { \
        ShowWarning("Wrong inventory index\n"); \
        script->reportsrc(st); \
        return false; \
    }

#define getND() \
    TBL_NPC *nd = map->id2nd(st->oid); \
    if (!nd) \
        return; \

#define getNDReturn(def) \
    TBL_NPC *nd = map->id2nd(st->oid); \
    if (!nd) \
        return def; \

#endif  // EVOL_MAP_SCRIPTDEFINES
