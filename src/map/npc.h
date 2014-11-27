// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_NPC
#define EVOL_MAP_NPC

struct npc_data* enpc_checknear(struct map_session_data* sd, struct block_list* bl);

void enpc_parse_unknown_mapflag(const char *name, char *w3, char *w4, const char* start,
                                const char* buffer, const char* filepath, int *retval);

#endif  // EVOL_MAP_NPC
