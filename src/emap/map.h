// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_MAP
#define EVOL_MAP_MAP

int emap_addflooritem_post(int retVal,
                           const struct block_list *bl,
                           struct item *item,
                           int *amount,
                           int16 *m,
                           int16 *x,
                           int16 *y,
                           int *first_charid,
                           int *second_charid,
                           int *third_charid,
                           int *flags);
void emap_online_list(int fd);

#endif  // EVOL_MAP_MAP
