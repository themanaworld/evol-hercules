// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_CHAR_CONFIG
#define EVOL_CHAR_CONFIG

void config_final(void);
void config_min_char_class(const char *val);
void config_max_char_class(const char *val);
void config_min_look(const char *val);
void config_max_look(const char *val);

extern int min_char_class;
extern int max_char_class;
extern int min_look;
extern int max_look;

#endif  // EVOL_CHAR_CONFIG
