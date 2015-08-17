// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_ITEMD
#define EVOL_MAP_ITEMD

struct ItemdExt *itemd_get_by_item(struct item *item);
struct ItemdExt *itemd_get(struct item_data *item);
struct ItemdExt *itemd_create(void);

#endif  // EVOL_MAP_MAPD
