#!/bin/bash

# MAX_SKILL    1478  + 22 = 1500
# MAX_SKILL_ID 10015 + 22 + 9963 = 20022
# SC_MAX       642   + 5  = 647
# SI_MAX       966   + 5  = 971
# MAX_EVOL_SKILLS           22
# EVOL_FIRST_SKILL          20000
# OLD_MAX_SKILL_DB          1478

# can be used for custom skill id: 10016 - 10036

export VARS=" -DOLD_MAX_SKILL_DB=1478 -DMAX_SKILL=1500 -DMAX_SKILL_ID=20022 -DMAX_EVOL_SKILLS=22 -DEVOL_FIRST_SKILL=20000 -DSC_MAX=647 -DSI_MAX=971"
export CPPFLAGS="${VARS}"