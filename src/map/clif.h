// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_CLIF
#define EVOL_MAP_CLIF

void eclif_quest_send_list(struct map_session_data *sd);
void eclif_quest_add(struct map_session_data *sd, struct quest *qd);
void eclif_charnameack(int *fdPtr, struct block_list *bl);
void eclif_getareachar_unit_post(struct map_session_data* sd, struct block_list *bl);
void eclif_authok_post(struct map_session_data *sd);
void eclif_sendlook(struct block_list *bl, int *id, int *type,
                    int *val, int *val2, enum send_target *target);
bool eclif_send(const void* buf, int *len, struct block_list* bl, enum send_target *type);
void eclif_set_unit_idle(struct block_list* bl, struct map_session_data *tsd,
                         enum send_target *target);

#endif  // EVOL_MAP_CLIF
