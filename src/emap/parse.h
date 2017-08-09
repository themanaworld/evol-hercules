// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_PARSE
#define EVOL_MAP_PARSE

void map_parse_version(int fd);
void map_parse_join_channel(int fd);
void map_parse_part_channel(int fd);
void map_parse_pet_say(int fd);
void map_parse_pet_emote(int fd);
void map_parse_set_status(int fd);
void map_parse_get_online_list(int fd);
void map_parse_pet_move(int fd);
void map_parse_pet_dir(int fd);
void map_parse_homun_say(int fd);
void map_parse_homun_emote(int fd);
void map_parse_homun_dir(int fd);
void map_clif_parse_useitem2(int fd);

#endif  // EVOL_MAP_PARSE
