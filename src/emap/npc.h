// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_NPC
#define EVOL_MAP_NPC

void enpc_parse_unknown_mapflag_pre(const char **namePtr,
                                    const char **w3Ptr,
                                    const char **w4Ptr,
                                    const char **startPtr,
                                    const char **bufferPtr,
                                    const char **filepathPtr,
                                    int **retval);

int enpc_buysellsel_pre(TBL_PC **sdPtr,
                        int *id,
                        int *type);

bool enpc_db_checkid_pre(const int *idPtr);

#endif  // EVOL_MAP_NPC
