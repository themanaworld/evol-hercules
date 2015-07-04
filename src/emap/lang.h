// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_LANG
#define EVOL_MAP_LANG

extern struct DBMap *translate_db;

void do_init_langs (void);
void do_final_langs(void);
const char* lang_trans(const char *str, int lng, int flg);
const char* lang_pctrans(const char *str, TBL_PC *sd);
int lang_getId(const char *str);

#endif  // EVOL_MAP_LANG
