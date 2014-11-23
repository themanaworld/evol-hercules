// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_CLIF
#define EVOL_MAP_CLIF

void eclif_quest_send_list(struct map_session_data *sd);
void eclif_quest_add(struct map_session_data *sd, struct quest *qd);
void eclif_charnameack(int *fdPtr, struct block_list *bl);
void eclif_getareachar_unit_post(struct map_session_data* sd, struct block_list *bl);

#endif  // EVOL_MAP_CLIF
