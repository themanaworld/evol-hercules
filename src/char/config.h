// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_CHAR_CONFIG
#define EVOL_CHAR_CONFIG

void config_final(void);
void config_inter_server_ip(const char *val);
void config_max_char_class(const char *val);

extern char *inter_server_ip;
extern int max_char_class;

#endif  // EVOL_CHAR_CONFIG
