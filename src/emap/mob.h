// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_MOB
#define EVOL_MAP_MOB

int emob_deleteslave_sub(struct block_list *bl, va_list ap);
void emob_read_db_additional_fields(struct mob_db *entry,
                                    int *classPtr,
                                    config_setting_t *it,
                                    int *nPtr, const char *source);

#endif  // EVOL_MAP_MOB
