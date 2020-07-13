// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_ENUM_ESITYPE
#define EVOL_MAP_ENUM_ESITYPE

#ifndef OLD_SI_MAX
#define OLD_SI_MAX 0
#error "vars.sh did not define OLD_SI_MAX"
#endif

enum esi_type
{
    SI_PHYSICAL_SHIELD = OLD_SI_MAX, // defined in vars.sh
    SI_EVOL_INCSTR,
    SI_EVOL_INCAGI,
    SI_EVOL_INCVIT,
    SI_EVOL_INCINT,
    SI_EVOL_INCDEX,
    SI_EVOL_INCLUK,
    SI_EVOL_INCHIT,
    SI_EVOL_INCFLEE,
    SI_EVOL_WALKSPEED,
    SI_EVOL_INCMHPRATE,
    SI_EVOL_INCMSPRATE,
};

#endif  // EVOL_MAP_ENUM_ESITYPE
