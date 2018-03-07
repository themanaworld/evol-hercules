// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_ITEMDB
#define EVOL_MAP_ITEMDB

bool eitemdb_is_item_usable_pre(struct item_data **itemPtr);
void eitemdb_readdb_additional_fields_pre(int *itemid,
                                          struct config_setting_t **itPtr,
                                          int *n,
                                          const char **sourcePtr);
void edestroy_item_data_pre(struct item_data **selfPtr,
                            int *free_selfPtr);

int eitemdb_isidentified(int nameid);

int eitemdb_isidentified2(struct item_data *item);

#endif  // EVOL_MAP_ITEMDB
