// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_CLIF
#define EVOL_MAP_CLIF

void eclif_quest_send_list_pre(TBL_PC **sdPtr);
void eclif_quest_add(TBL_PC *sd,
                     struct quest *qd);
void eclif_quest_add_pre(TBL_PC **sdPtr,
                         struct quest **qdPtr);
void eclif_blname_ack_pre(int *fdPtr,
                          struct block_list **blPtr);
void eclif_blname_ack_pre_sub(int *fdPtr,
                              struct block_list **blPtr);
void eclif_homname_ack_pre(int *fdPtr, struct block_list **blPtr);
void eclif_mername_ack_pre(int *fdPtr, struct block_list **blPtr);
void eclif_petname_ack_pre(int *fdPtr, struct block_list **blPtr);
void eclif_elemname_ack_pre(int *fdPtr, struct block_list **blPtr);
void eclif_charnameupdate_pre(struct map_session_data **ssdPtr);
void eclif_getareachar_unit_post(TBL_PC *sd,
                                 struct block_list *bl);
bool eclif_spawn_post(bool retVal,
                      struct block_list *bl);
void eclif_sendlook_pre(struct block_list **blPtr,
                        int *id,
                        int *type,
                        int *val,
                        int *val2,
                        enum send_target *target);
bool eclif_send_pre(const void **bufPtr,
                    int *len,
                    struct block_list **blPtr,
                    enum send_target *type);
void eclif_set_unit_idle_pre(struct block_list **blPtr,
                             TBL_PC **tsdPtr,
                             enum send_target *target);
int eclif_send_actual_pre(int *fd,
                          void **bufPtr,
                          int *len);
void eclif_authok_post(TBL_PC *sd);
void eclif_changemap_post(TBL_PC *sd,
                          short m,
                          int x,
                          int y);
void eclif_set_unit_idle_post(struct block_list *bl,
                              TBL_PC *tsd,
                              enum send_target target);
void eclif_set_unit_walking_post(struct block_list *bl,
                                 TBL_PC *tsd,
                                 struct unit_data *ud,
                                 enum send_target target);
void eclif_move_post(struct unit_data *ud);
void eclif_parse_LoadEndAck_pre(int *fdPtr,
                                struct map_session_data **sdPtr);
void eclif_parse_LoadEndAck_post(int fd,
                                 struct map_session_data *sd);
void eclif_changelook2(struct block_list *bl,
                       int type,
                       int val,
                       struct item_data *id,
                       int n);
void eclif_getareachar_item_pre(struct map_session_data **sdPtr,
                                struct flooritem_data **fitemPtr);
void eclif_dropflooritem_pre(struct flooritem_data **fitemPtr);
void eclif_sendbgemblem_area_pre(struct map_session_data **sdPtr);
void eclif_sendbgemblem_single_pre(int *fdPtr,
                                   struct map_session_data **sdPtr);
void eclif_disp_message_pre(struct block_list **srcPtr,
                            const char **mesPtr,
                            enum send_target *targetPtr);

void eclif_addcards_post(struct EQUIPSLOTINFO *buf,
                         struct item *item);
void eclif_useskill(struct block_list* bl,
                    int src_id,
                    int dst_id,
                    int dst_x,
                    int dst_y,
                    uint16 skill_id,
                    uint16 skill_lv,
                    int casttime);
void eclif_skillinfoblock_pre(struct map_session_data **sdPtr);
void eclif_addskill_pre(struct map_session_data **sdPtr,
                        int *idPtr);
void eclif_skillinfo_pre(struct map_session_data **sdPtr,
                         int *skill_idPtr,
                         int *infPtr);
void eclif_parse_WalkToXY(int fd,
                          struct map_session_data *sd) __attribute__((nonnull (2)));
void eclif_party_info_post(struct party_data *p,
                           struct map_session_data *sd);
void eclif_parse_NpcStringInput(int fd,
                                struct map_session_data* sd) __attribute__((nonnull (2)));
void eclif_rodex_icon_pre(int *fdPtr,
                          bool *showPtr);
#endif  // EVOL_MAP_CLIF
