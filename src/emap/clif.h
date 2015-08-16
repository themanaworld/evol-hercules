// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_CLIF
#define EVOL_MAP_CLIF

void eclif_quest_send_list(TBL_PC *sd);
void eclif_quest_add(TBL_PC *sd, struct quest *qd);
void eclif_charnameack(int *fdPtr, struct block_list *bl);
void eclif_getareachar_unit_post(TBL_PC* sd, struct block_list *bl);
void eclif_sendlook(struct block_list *bl, int *id, int *type,
                    int *val, int *val2, enum send_target *target);
bool eclif_send(const void* buf, int *len, struct block_list* bl, enum send_target *type);
void eclif_set_unit_idle(struct block_list* bl, TBL_PC *tsd,
                         enum send_target *target);
int eclif_send_actual(int *fd, void *buf, int *len);

void eclif_authok_post(TBL_PC *sd);
void eclif_changemap_post(TBL_PC *sd, short *m, int *x, int *y);
void eclif_set_unit_idle_post(struct block_list* bl, TBL_PC *tsd,
                              enum send_target *target);
void eclif_set_unit_walking(struct block_list* bl, TBL_PC *tsd,
                            struct unit_data* ud, enum send_target *target);
void eclif_move(struct unit_data *ud);
void eclif_parse_LoadEndAck_pre(int *fdPtr,
                                struct map_session_data *sd);
void eclif_changelook2(struct block_list *bl, int type, int val,
                       struct item_data *id, int n);
void eclif_getareachar_item(struct map_session_data *sd, struct flooritem_data *fitem);

#endif  // EVOL_MAP_CLIF