// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_MOB
#define EVOL_MAP_MOB

#define MD_SURVIVE_WITHOUT_MASTER 0x0040000

int emob_deleteslave_sub(struct block_list *bl, va_list ap);
void emob_read_db_additional_fields(struct mob_db *entry,
                                    struct config_setting_t *it,
                                    int *nPtr, const char *source);
int emob_read_db_mode_sub_post(int retVal,
                               struct mob_db *entry,
                               struct config_setting_t *t);

#endif  // EVOL_MAP_MOB
