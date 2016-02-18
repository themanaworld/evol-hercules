// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_ITEMDB
#define EVOL_MAP_ITEMDB

bool eitemdb_is_item_usable(struct item_data *item);
void eitemdb_readdb_additional_fields(int *itemid, struct config_setting_t *it, int *n, const char *source);
void edestroy_item_data(struct item_data* self, int *free_selfPtr);

#endif  // EVOL_MAP_ITEMDB
