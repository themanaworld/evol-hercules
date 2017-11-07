// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_STATUS
#define EVOL_MAP_STATUS

int estatus_init_post(int retVal, bool minimal);
void estatus_set_viewdata_post(struct block_list *bl,
                               int class_);
void estatus_read_job_db_sub_post(int idx,
                                  const char *name,
                                  struct config_setting_t *jdb);
int estatus_calc_pc__post(int retVal,
                          struct map_session_data *sd,
                          enum e_status_calc_opt opt);
void estatus_calc_pc_additional_pre(struct map_session_data **sdPtr,
                                    enum e_status_calc_opt *optPtr);
unsigned short estatus_calc_speed_post(unsigned short retVal,
                                       struct block_list *bl,
                                       struct status_change *sc,
                                       int speed);

defType estatus_calc_def_post(defType retVal,
                              struct block_list *bl,
                              struct status_change *sc,
                              int def, bool viewable);

short estatus_calc_fix_aspd_post(short retVal,
                                 struct block_list *bl,
                                 struct status_change *sc,
                                 int aspd);

int estatus_change_start_post(int retVal,
                              struct block_list *src,
                              struct block_list *bl,
                              enum sc_type type,
                              int rate, int val1, int val2,
                              int val3, int val4, int tick, int flag);

int estatus_change_end__post(int retVal,
                             struct block_list* bl,
                             enum sc_type type, int tid,
                             const char* file, int line);

#endif  // EVOL_MAP_STATUS
