// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_COMMON_UTILS_STRUTIL
#define EVOL_COMMON_UTILS_STRUTIL

struct strutil_data *strutil_split(const char *str,
                                   const char separator,
                                   const int len);
void strutil_free(struct strutil_data *data);

#endif  // EVOL_COMMON_UTILS_STRUTIL
