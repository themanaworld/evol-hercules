// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_PC
#define EVOL_MAP_PC

void send_npccommand (struct map_session_data *sd, int npcId, int cmd);
void send_npccommand2 (struct map_session_data *sd, int npcId, int cmd, int id, int x, int y);

#endif  // EVOL_MAP_PC
