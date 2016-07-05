// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_SKILL
#define EVOL_MAP_SKILL

int eskill_get_index_post(int retVal,
                          uint16 skill_id);

int eskill_check_condition_castend_post(int retVal,
                                        TBL_PC* sd,
                                        uint16 skill_id,
                                        uint16 skill_lv);

bool eskill_castend_nodamage_id_unknown(struct block_list *src,
                                        struct block_list *bl,
                                        uint16 *skill_id,
                                        uint16 *skill_lv,
                                        int64 *tick,
                                        int *flag);

void eskill_additional_effect_unknown(struct block_list* src,
                                      struct block_list *bl,
                                      uint16 *skill_id,
                                      uint16 *skill_lv,
                                      int *attack_type,
                                      int *dmg_lv,
                                      int64 *tick);

void eskill_counter_additional_effect_unknown(struct block_list* src,
                                              struct block_list *bl,
                                              uint16 *skill_id,
                                              uint16 *skill_lv,
                                              int *attack_type,
                                              int64 *tick);

void eskill_attack_combo2_unknown(int *attack_type,
                                  struct block_list *src,
                                  struct block_list *dsrc,
                                  struct block_list *bl,
                                  uint16 *skill_id,
                                  uint16 *skill_lv,
                                  int64 *tick,
                                  int *flag,
                                  int *combo);

void eskill_attack_post_unknown(int *attack_type,
                                struct block_list *src,
                                struct block_list *dsrc,
                                struct block_list *bl,
                                uint16 *skill_id,
                                uint16 *skill_lv,
                                int64 *tick,
                                int *flag);

void eskill_timerskill_notarget_unknown(int tid,
                                        int64 tick,
                                        struct block_list *src,
                                        struct unit_data *ud,
                                        struct skill_timerskill *skl);

void eskill_unitsetting1_unknown(struct block_list *src,
                                 uint16 *skill_id,
                                 uint16 *skill_lv,
                                 int16 *x,
                                 int16 *y,
                                 int *flag,
                                 int *val1,
                                 int *val2,
                                 int *val3);

void eskill_unit_onplace_unknown(struct skill_unit *src,
                                 struct block_list *bl,
                                 int64 *tick);

void eskill_check_condition_castend_unknown(struct map_session_data *sd,
                                            uint16 *skill_id,
                                            uint16 *skill_lv);

void eskill_get_requirement_unknown(struct status_change *sc,
                                    struct map_session_data *sd,
                                    uint16 *skill_id,
                                    uint16 *skill_lv,
                                    struct skill_condition *req);

bool eskill_castend_pos2_unknown(struct block_list* src,
                                 int *x,
                                 int *y,
                                 uint16 *skill_id,
                                 uint16 *skill_lv,
                                 int64 *tick,
                                 int *flag);

#endif  // EVOL_MAP_SKILL
