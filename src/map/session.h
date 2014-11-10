// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_SESSION
#define EVOL_MAP_SESSION

struct SessionExt;

struct SessionExt *session_get(int fd);
struct SessionExt *session_create(void);

#endif  // EVOL_MAP_SESSION