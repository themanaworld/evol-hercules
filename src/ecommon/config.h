// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_COMMON_CONFIG
#define EVOL_COMMON_CONFIG

void config_default_map(const char *val);
void config_default_x(const char *val);
void config_default_y(const char *val);
void common_config_final(void);

extern const char *default_map;
extern int default_x;
extern int default_y;

#endif  // EVOL_COMMON_CONFIG
