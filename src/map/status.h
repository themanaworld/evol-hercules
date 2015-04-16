// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_STATUS
#define EVOL_MAP_STATUS

void status_init(void);
void estatus_set_viewdata_post(struct block_list *bl, int *class_);
void estatus_read_job_db_sub(int *idxPtr, const char *name, config_setting_t *jdb);

#endif  // EVOL_MAP_STATUS
