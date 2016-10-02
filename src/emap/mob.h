// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_MOB
#define EVOL_MAP_MOB

#define MD_SURVIVE_WITHOUT_MASTER 0x0040000

int emob_deleteslave_sub_pre(struct block_list **blPtr,
                             va_list ap);
void emob_read_db_additional_fields_pre(struct mob_db **entryPtr,
                                        struct config_setting_t **itPtr,
                                        int *nPtr,
                                        const char **sourcePtr);
uint32 emob_read_db_mode_sub_post(uint32 retVal,
                                  struct mob_db *entry,
                                  struct config_setting_t *t);
struct mob_data *emob_spawn_dataset_post(struct mob_data *retVal,
                                         struct spawn_data *data);
int emob_dead_pre(struct mob_data **mdPtr,
                  struct block_list **srcPtr,
                  int *typePtr);

#endif  // EVOL_MAP_MOB
