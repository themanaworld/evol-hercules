// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_NPC
#define EVOL_MAP_NPC

void enpc_parse_unknown_mapflag_pre(const char **namePtr,
                                    const char **w3Ptr,
                                    const char **w4Ptr,
                                    const char **startPtr,
                                    const char **bufferPtr,
                                    const char **filepathPtr,
                                    int **retval);

int enpc_buysellsel_pre(TBL_PC **sdPtr,
                        int *id,
                        int *type);

bool enpc_db_checkid_pre(const int *idPtr);

bool enpc_duplicate_script_sub_pre(struct npc_data **ndPtr,
                                   const struct npc_data **sndPtr,
                                   int *xsPtr,
                                   int *ysPtr,
                                   int *optionsPtr);

void enpc_set_var_num(TBL_NPC *const npc,
                      const char *var,
                      const int val);

int enpc_get_var_num(const TBL_NPC *const npc,
                     const char *var);

int enpc_unload_pre(struct npc_data** ndPtr,
                    bool *singlePtr);

struct view_data *enpc_get_viewdata_post(struct view_data *retVal,
                                         int class_);

#endif  // EVOL_MAP_NPC
