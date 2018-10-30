// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_ATCOMMAND
#define EVOL_MAP_ATCOMMAND

const char* eatcommand_msgsd_pre(struct map_session_data **sdPtr,
                                 int *msgPtr);
const char* eatcommand_msgfd_pre(int *fdPtr,
                                 int *msgPtr);

#define ACMD0(x) bool atcommand_ ## x (const int fd __attribute__ ((unused)), \
    struct map_session_data* sd __attribute__ ((unused)), \
    const char* command __attribute__ ((unused)), \
    const char* message __attribute__ ((unused)), struct AtCommandInfo *info __attribute__ ((unused)))
#define ACMD1(x) bool atcommand_ ## x (const int fd __attribute__ ((unused)), \
    struct map_session_data* sd __attribute__ ((unused)), \
    const char* command __attribute__ ((unused)), \
    const char* message, struct AtCommandInfo *info __attribute__ ((unused)))
#define ACMD2(x) bool atcommand_ ## x (const int fd, \
    struct map_session_data* sd, \
    const char* command __attribute__ ((unused)), \
    const char* message, \
    struct AtCommandInfo *info)
#define ACMD3(x) bool atcommand_ ## x (const int fd __attribute__ ((unused)), \
    struct map_session_data* sd, \
    const char* command __attribute__ ((unused)), \
    const char* message __attribute__ ((unused)), \
    struct AtCommandInfo *info __attribute__ ((unused)))
#define ACMD4(x) bool atcommand_ ## x (const int fd __attribute__ ((unused)), \
    struct map_session_data* sd, \
    const char* command __attribute__ ((unused)), \
    const char* message, \
    struct AtCommandInfo *info __attribute__ ((unused)))

ACMD2(setSkill);
ACMD2(slide);
ACMD3(hugo);
ACMD3(linus);
ACMD1(mapExit);
ACMD1(serverExit);
ACMD0(log);
ACMD4(tee);

#endif  // EVOL_MAP_ATCOMMAND
