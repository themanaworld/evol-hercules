// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_PARSE
#define EVOL_MAP_PARSE

void map_parse_version(int fd);
void sample_packet0f3(int fd);

int epc_parse_setparam_pre(struct map_session_data *sd, int *type, int *val);
int epc_parse_readparam_pre(struct map_session_data* sd, int *type);

#endif  // EVOL_MAP_PARSE
