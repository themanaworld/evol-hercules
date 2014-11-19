// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_SCRIPTDEFINES
#define EVOL_MAP_SCRIPTDEFINES

#define getDataReturn(def) \
    if (!st->rid) \
    { \
        script_pushint(st, def); \
        return true; \
    } \
    TBL_PC *sd = script->rid2sd(st); \
    if (!sd) \
    { \
        script_pushint(st, def); \
        return true; \
    } \
    struct SessionExt *data = session_get(sd->fd)

#define getData() \
    if (!st->rid) \
        return true; \
    TBL_PC *sd = script->rid2sd(st); \
    if (!sd) \
        return true; \
    struct SessionExt *data = session_get(sd->fd)

#define getSD() \
    TBL_PC *sd = script->rid2sd(st); \
    if (!sd) \
        return true

#define getSDReturn(def) \
    TBL_PC *sd = script->rid2sd(st); \
    if (!sd) \
    { \
        script_pushint(st, def); \
        return true; \
    }

#endif  // EVOL_MAP_SCRIPTDEFINES
