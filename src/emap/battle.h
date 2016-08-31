// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_BATTLE
#define EVOL_MAP_BATTLE

bool ebattle_check_arrows_post(bool retVal,
                               struct map_session_data *sd);
struct Damage ebattle_calc_weapon_attack_post(struct Damage retVal,
                                              struct block_list *src,
                                              struct block_list *target,
                                              uint16 skill_id,
                                              uint16 skill_lv,
                                              int wflag);

#endif  // EVOL_MAP_BATTLE
