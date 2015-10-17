// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_PC
#define EVOL_MAP_PC

enum VarConst
{
    Const_ClientVersion = 10000
};

int epc_readparam_pre(TBL_PC* sd, int *type);

int epc_setregistry(TBL_PC *sd, int64 *reg, int *val);

void epc_equipitem_pos(TBL_PC *sd, struct item_data *id, int *nPtr, int *posPtr);

void epc_unequipitem_pos(TBL_PC *sd, int *nPtr, int *posPtr);

bool epc_can_attack (TBL_PC *sd, int *target_id);

int epc_takeitem(TBL_PC *sd, TBL_ITEM *fitem);

void epc_validate_levels(void);

int epc_isequip_post(int retVal, struct map_session_data *sd, int *nPtr);

int epc_useitem_post(int retVal, struct map_session_data *sd, int *nPtr);

int epc_equipitem_post(int retVal, struct map_session_data *sd,
                       int *nPtr, int *data);

int epc_unequipitem_post(int retVal, struct map_session_data *sd,
                         int *nPtr, int *data);

int epc_check_job_name(const char *name);

int epc_setnewpc_post(int retVal, struct map_session_data *sd,
                      int *account_id, int *char_id, int *login_id1,
                      unsigned int *client_tick, int *sex, int *fd);

int epc_additem_post(int retVal, struct map_session_data *sd,
                     struct item *item_data, int *amountPtr,
                     e_log_pick_type *log_type);

int epc_delitem_pre(struct map_session_data *sd, int *nPtr, int *amountPtr,
                    int *typePtr, short *reasonPtr,
                    e_log_pick_type *log_type);

int epc_delitem_post(int retVal, struct map_session_data *sd, int *nPtr, int *amountPtr,
                     int *typePtr, short *reasonPtr,
                     e_log_pick_type *log_type);

bool epc_can_insert_card_into_post(bool retVal, struct map_session_data* sd,
                                   int *idx_card, int *idx_equip);

int epc_dropitem_pre(struct map_session_data *sd, int *nPtr, int *amountPtr);

int epc_dropitem_post(int retVal, struct map_session_data *sd, int *nPtr, int *amountPtr);

int epc_takeitem_pre(struct map_session_data *sd, struct flooritem_data *fitem);

int epc_takeitem_post(int retVal, struct map_session_data *sd, struct flooritem_data *fitem);

int epc_insert_card_pre(struct map_session_data* sd, int *idx_card, int *idx_equip);

int epc_insert_card_post(int retVal, struct map_session_data* sd, int *idx_card, int *idx_equip);

#endif  // EVOL_MAP_PC
