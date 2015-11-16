// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_ATCOMMAND
#define EVOL_MAP_ATCOMMAND

const char* eatcommand_msgsd(struct map_session_data *sd, int *msgPtr);
const char* eatcommand_msgfd(int *fdPtr, int *msgPtr);

#define ACMD2(x) bool atcommand_ ## x (const int fd, struct map_session_data* sd, const char* command __attribute__ ((unused)), const char* message, struct AtCommandInfo *info)

ACMD2(setSkill);
ACMD2(slide);

#endif  // EVOL_MAP_ATCOMMAND
