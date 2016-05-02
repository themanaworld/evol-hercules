// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_SCRIPT
#define EVOL_MAP_SCRIPT

int escript_reload_pre(void);
void escript_load_translations_pre(void);
void escript_load_parameters_pre(void);
void eset_reg_npcscope_num_pre(struct script_state **stPtr,
                               struct reg_db **nPtr,
                               int64 *numPtr,
                               const char **namePtr,
                               int *val);
int eget_val_npcscope_num_pre(struct script_state **stPtr,
                              struct reg_db **nPtr,
                              struct script_data **dataPtr);
void eset_reg_npcscope_str_pre(struct script_state **stPtr,
                               struct reg_db **nPtr,
                               int64 *num,
                               const char **namePtr,
                               const char **strPtr);
char *eget_val_npcscope_str_pre(struct script_state **stPtr,
                                struct reg_db **nPtr,
                                struct script_data **dataPtr);
void script_run_item_amount_script(TBL_PC *sd,
                                   struct script_code *itemScript,
                                   int itemId,
                                   int amount);
void script_run_card_script(TBL_PC *sd,
                            struct script_code *itemScript,
                            int itemId,
                            int cardId);

#endif  // EVOL_MAP_SCRIPT
