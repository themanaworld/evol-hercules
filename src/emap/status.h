// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_STATUS
#define EVOL_MAP_STATUS

void status_init(void);
void estatus_set_viewdata_post(struct block_list *bl, int *class_);
void estatus_read_job_db_sub(int *idxPtr, const char *name, config_setting_t *jdb);
int estatus_calc_pc_(int retVal, struct map_session_data* sd, enum e_status_calc_opt *opt);
int estatus_calc_pc_additional(struct map_session_data* sd, enum e_status_calc_opt *opt);

#endif  // EVOL_MAP_STATUS
