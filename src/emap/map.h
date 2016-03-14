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
int emap_getcellp(struct map_data* m,
                  const struct block_list *bl,
                  int16 *xPtr, int16 *yPtr,
                  cell_chk *cellchkPtr);
struct mapcell emap_gat2cell(int *gatPtr);
int emap_cell2gat(struct mapcell *cellPtr);
void emap_setgatcell(int16 *mPtr,
                     int16 *xPtr, int16 *yPtr,
                     int *gatPtr);
bool emap_iwall_set(int16 *m,
                    int16 *x, int16 *y,
                    int *size,
                    int8 *dir,
                    bool *shootable,
                    const char* wall_name);
void emap_iwall_get(struct map_session_data *sd);
void emap_iwall_remove(const char *name);
bool emap_iwall_set2(int m,
                     int layer,
                     int x1, int y1,
                     int x2, int y2,
                     int mask,
                     const char *name);
void map_alwaysVisible_add(const struct block_list *bl);
void map_alwaysVisible_delete(const struct block_list *bl);
bool map_alwaysVisible_find(const struct block_list *bl);
void map_alwaysVisible_send(TBL_PC *sd);
void edo_final_maps(void);
void map_clear_data(void);

#endif  // EVOL_MAP_MAP
