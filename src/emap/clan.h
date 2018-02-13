// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_CLAN
#define EVOL_MAP_CLAN

bool eclan_join_post(bool retVal,
                     struct map_session_data *sd,
                     int clan_id);

bool eclan_leave_post(bool retVal,
                      struct map_session_data *sd,
                      bool first);

#endif  // EVOL_MAP_CLAN
