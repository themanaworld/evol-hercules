// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_ENUM_ESCTYPE
#define EVOL_MAP_ENUM_ESCTYPE

#ifndef OLD_SC_MAX
#define OLD_SC_MAX 0
#error "vars.sh did not define OLD_SC_MAX"
#endif

typedef enum esc_type
{
    SC_PHYSICAL_SHIELD = OLD_SC_MAX, // defined in vars.sh
} esc_type;

#endif  // EVOL_MAP_ENUM_ESCTYPE
