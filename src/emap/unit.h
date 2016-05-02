// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_UNIT
#define EVOL_MAP_UNIT

int eunit_can_move_pre(struct block_list **blPtr);
int eunit_walktoxy_pre(struct block_list **blPtr,
                       short *x,
                       short *y,
                       int *flagPtr);

#endif  // EVOL_MAP_UNIT
