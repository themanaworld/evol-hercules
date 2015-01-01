// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_NPC
#define EVOL_MAP_NPC

void enpc_parse_unknown_mapflag(const char *name, char *w3, char *w4, const char* start,
                                const char* buffer, const char* filepath, int *retval);

int enpc_buysellsel(struct map_session_data* sd, int *id, int *type);

bool enpc_db_checkid(int *idPtr);

#endif  // EVOL_MAP_NPC
