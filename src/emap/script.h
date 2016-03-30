// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_SCRIPT
#define EVOL_MAP_SCRIPT

int escript_reload(void);
void escript_load_translations(void);
void escript_load_parameters(void);
void eset_reg_npcscope_num(struct script_state* st, struct reg_db *n, int64 *num, const char* name, int *val);
int eget_val_npcscope_num(struct script_state* st, struct reg_db *n, struct script_data* data);
void eset_reg_npcscope_str(struct script_state* st, struct reg_db *n, int64 *num, const char* name, const char *str);
char *eget_val_npcscope_str(struct script_state* st, struct reg_db *n, struct script_data* data);
void script_run_item_amount_script(TBL_PC *sd, struct script_code *itemScript, int itemId, int amount);
void script_run_card_script(TBL_PC *sd, struct script_code *itemScript, int itemId, int cardId);

#endif  // EVOL_MAP_SCRIPT
