// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_PC
#define EVOL_MAP_PC

void send_npccommand (struct map_session_data *sd, int npcId, int cmd);
void send_npccommand2 (struct map_session_data *sd, int npcId, int cmd, int id, int x, int y);
void send_local_message(int fd, struct block_list* bl, const char* msg);
void send_changelook(int fd, int id, int type, int val);
void send_mapmask(int fd, int mask);
void send_mapmask_brodcast(const int map, const int mask);

#endif  // EVOL_MAP_PC
