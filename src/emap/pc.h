// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_PC
#define EVOL_MAP_PC

enum VarConst
{
    Const_ClientVersion = 10000
};

int64 epc_readparam_pre(const TBL_PC **sdPtr,
                        int *type);

int epc_setparam_pre(TBL_PC **sdPtr,
                     int *type,
                     int64 *val);

int epc_setregistry_pre(TBL_PC **sdPtr,
                        int64 *reg,
                        int *val);

void epc_equipitem_pos_pre(TBL_PC **sdPtr,
                           struct item_data **idPtr,
                           int *nPtr,
                           int *posPtr);

void epc_unequipitem_pos_pre(TBL_PC **sdPtr,
                             int *nPtr,
                             int *posPtr);

bool epc_can_attack_pre(TBL_PC **sdPtr,
                        int *target_id);

void epc_validate_levels_pre(void);

int epc_isequip_post(int retVal,
                     struct map_session_data *sd,
                     int n);

int epc_useitem_post(int retVal,
                     struct map_session_data *sd,
                     int n);

int epc_equipitem_post(int retVal,
                       struct map_session_data *sd,
                       int n,
                       int data);

int epc_unequipitem_post(int retVal,
                         struct map_session_data *sd,
                         int n,
                         int data);

int epc_check_job_name_pre(const char **namePtr);

int epc_setnewpc_post(int retVal,
                      struct map_session_data *sd,
                      int account_id,
                      int char_id,
                      int login_id1,
                      unsigned int client_tick,
                      int sex,
                      int fd);

int epc_additem_post(int retVal,
                     struct map_session_data *sd,
                     const struct item *item_data,
                     int amount,
                     e_log_pick_type log_type);

int epc_delitem_pre(struct map_session_data **sdPtr,
                    int *nPtr,
                    int *amountPtr,
                    int *typePtr,
                    short *reasonPtr,
                    e_log_pick_type *log_type);

int epc_delitem_post(int retVal,
                     struct map_session_data *sd,
                     int n,
                     int amount,
                     int type,
                     short reason,
                     e_log_pick_type log_type);

bool epc_can_insert_card_into_post(bool retVal,
                                   struct map_session_data* sd,
                                   int idx_card,
                                   int idx_equip);

int epc_dropitem_pre(struct map_session_data **sdPtr,
                     int *nPtr,
                     int *amountPtr);

int epc_dropitem_post(int retVal,
                      struct map_session_data *sd,
                      int n,
                      int amount);

int epc_takeitem_pre(struct map_session_data **sdPtr,
                     struct flooritem_data **fitemPtr);

int epc_takeitem_post(int retVal,
                      struct map_session_data *sd,
                      struct flooritem_data *fitem);

int epc_insert_card_pre(struct map_session_data **sdPtr,
                        int *idx_card,
                        int *idx_equip);

int epc_insert_card_post(int retVal,
                         struct map_session_data* sd,
                         int idx_card,
                         int idx_equip);

bool epc_can_Adopt_pre(struct map_session_data **p1_sdPtr,
                       struct map_session_data **p2_sdPtr,
                       struct map_session_data **b_sdPtr);

bool epc_adoption_pre(struct map_session_data **p1_sdPtr,
                      struct map_session_data **p2_sdPtr,
                      struct map_session_data **b_sdPtr);

bool epc_process_chat_message_pre(struct map_session_data **sdPtr,
                                  const char **messagePtr);

int epc_dead_post(int retVal,
                  struct map_session_data *sd,
                  struct block_list *src);

int epc_jobchange(struct map_session_data *sd,
                  int job,
                  int upper);

void epc_calc_skilltree_clear_pre(struct map_session_data **sdPtr);

void epc_calc_skilltree_bonus_pre(struct map_session_data **sdPtr,
                                  int *classidxPtr);

void epc_checkbaselevelup_sc_pre(struct map_session_data **sdPtr);

bool epc_resetskill_job_pre(struct map_session_data** sdPtr,
                            int *indexPtr);

bool epc_isDeathPenaltyJob_pre(uint16 *jobPtr);

bool epc_read_skill_job_skip_pre(short *skill_idPtr,
                                 int *job_idPtr);

#endif  // EVOL_MAP_PC
