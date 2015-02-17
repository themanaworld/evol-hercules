// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_PARSE
#define EVOL_MAP_PARSE

void map_parse_version(int fd);
void map_parse_join_channel(int fd);
void map_parse_part_channel(int fd);
void map_parse_pet_say(int fd);
void map_parse_pet_emote(int fd);

#endif  // EVOL_MAP_PARSE
