// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/db.h"
#include "common/HPMi.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/battle.h"
#include "map/itemdb.h"
#include "map/map.h"
#include "map/pc.h"

#include "emap/permission.h"
#include "emap/send.h"
#include "emap/data/itemd.h"
#include "emap/data/mobd.h"
#include "emap/data/npcd.h"
#include "emap/data/session.h"
#include "emap/struct/itemdext.h"
#include "emap/struct/mobdext.h"
#include "emap/struct/npcdext.h"
#include "emap/struct/sessionext.h"
#include "emap/struct/walldata2.h"

struct mapcell2
{
    // terrain flags
    unsigned char
        walkable  : 1,
        shootable : 1,
        water     : 1;

    // dynamic flags
    unsigned char
        npc       : 1,
        basilica  : 1,
        landprotector : 1,
        novending : 1,
        nochat    : 1,
        icewall   : 1,
        noicewall : 1,

        wall      : 1,
        air       : 1;

#ifdef CELL_NOSTACK
    int cell_bl; //Holds amount of bls in this cell.
#endif
};

int emap_addflooritem_post(int retVal,
                           const struct block_list *bl,
                           struct item *item,
                           int *amount __attribute__ ((unused)),
                           int16 *m __attribute__ ((unused)),
                           int16 *x __attribute__ ((unused)),
                           int16 *y __attribute__ ((unused)),
                           int *first_charid __attribute__ ((unused)),
                           int *second_charid __attribute__ ((unused)),
                           int *third_charid __attribute__ ((unused)),
                           int *flags __attribute__ ((unused)))
{
    TBL_ITEM* fitem = (TBL_ITEM*)idb_get(map->id_db, retVal);
    if (fitem && fitem->cleartimer != INVALID_TIMER)
    {
        int timeout = battle->bc->flooritem_lifetime;
        struct ItemdExt *data = itemd_get_by_item(item);
        if (data)
            timeout = data->floorLifeTime;
        timer->delete(fitem->cleartimer, map->clearflooritem_timer);
        if (timeout >= 0)
            fitem->cleartimer = timer->add(timer->gettick() + timeout, map->clearflooritem_timer, fitem->bl.id, 0);
    }
    return retVal;
}

void emap_online_list(int fd)
{
    char *buf = aCalloc (1, 20000);
    char *ptr = buf;
    TBL_PC* sd;

    struct SessionExt *data1 = session_get(fd);
    if (!data1)
    {
        aFree(buf);
        return;
    }

    const time_t t = time(NULL);
    if (data1->onlinelistlasttime + 15 >= t)
    { // not more than 1 per 15 seconds
        data1->onlinelistlasttime = t;
        aFree(buf);
        return;
    }

    TBL_PC* ssd = (TBL_PC*)sockt->session[fd]->session_data;
    if (!ssd)
    {
        aFree(buf);
        return;
    }

    const bool showVersion = pc_has_permission(ssd, permission_show_client_version_flag);
    const int gpoupLevel = pc_get_group_level(ssd);
    data1->onlinelistlasttime = t;

    DBIterator* iter = db_iterator(map->pc_db);

    for (sd = dbi_first(iter); dbi_exists(iter); sd = dbi_next(iter))
    {
        if (!sd)
            continue;

        if (ptr - buf > 19500)
            break;

        if (pc_isinvisible(sd) && gpoupLevel < pc_get_group_level(sd))
            continue;

        struct SessionExt *data = session_get_bysd(sd);
        if (!data)
            continue;

        uint8 state = data->state;
        if (sd->status.sex == 1)
            state |= 128;
        else
            state = (state | 128) ^ 128;

        if (pc_has_permission(sd, permission_send_gm_flag))
            state |= 64;
        else
            state = (state | 64) ^ 64;

        *ptr = state;
        ptr ++;

        *ptr = sd->status.base_level;
        ptr ++;

        if (showVersion)
            *ptr = data->clientVersion;
        else
            *ptr = 0;
        ptr ++;

        strcpy(ptr, sd->status.name);
        ptr += strlen(sd->status.name);
        *ptr = 0;
        ptr ++;

    }
    dbi_destroy(iter);
    send_online_list(fd, buf, ptr - buf);
    aFree(buf);
}


// walk mask
// 0000 0x0 - normal
// 0001 0x1 - wall walk
// 0010 0x2 - water walk
// 0100 0x4 - air walk


static int getWalkMask(const struct block_list *bl)
{
    int walkMask = 0;
    if (bl->type == BL_NPC)
    {
        TBL_NPC *nd = (TBL_NPC*)bl;
        struct NpcdExt *ext = npcd_get(nd);
        if (ext)
            walkMask = ext->walkMask;
    }
    else if (bl->type == BL_MOB)
    {
        TBL_MOB *md = (TBL_MOB*)bl;
        struct MobdExt *ext = mobd_get_by_mob(md);
        if (ext)
            walkMask = ext->walkMask;
    }
    return walkMask;
}

static bool isWalkCell(const struct block_list *bl, struct mapcell2 cell)
{
    if (cell.walkable)
        return true;
    const int walkMask = getWalkMask(bl);
    // water check
    if (cell.water && walkMask & 0x2)
        return true;
    // air check
    if ((cell.air || cell.water) && walkMask & 0x4)
        return true;
    // wall check
    if (cell.wall && walkMask & 0x1)
        return true;
    // other checks
    return false;
}

static bool isWallCell(const struct block_list *bl, struct mapcell2 cell)
{
    if (cell.walkable || cell.shootable)
        return false;
    const int walkMask = getWalkMask(bl);
    // water check
    if (cell.water && walkMask & 0x2)
        return false;
    if ((cell.air || cell.water) && walkMask & 0x4)
        return false;
    // wall check
    if (cell.wall && walkMask & 0x1)
        return false;
    return true;
}

#define strangeCast(type, val) *((type*)(&val))

int emap_getcellp(struct map_data* m,
                  const struct block_list *bl,
                  int16 *xPtr, int16 *yPtr,
                  cell_chk *cellchkPtr)
{
    if (bl && m)
    {
        const int x = *xPtr;
        const int y = *yPtr;
        const cell_chk cellchk = *cellchkPtr;
        if (x < 0 || x >= m->xs - 1 || y < 0 || y >= m->ys - 1)
            return (cellchk == CELL_CHKNOPASS);

        struct mapcell2 cell = strangeCast(struct mapcell2, m->cell[x + y * m->xs]);

        if (cellchk == CELL_CHKWALL ||
            cellchk == CELL_CHKPASS ||
            cellchk == CELL_CHKREACH ||
            cellchk == CELL_CHKNOPASS ||
            cellchk == CELL_CHKNOREACH)
        {
            bool res;
            switch (cellchk)
            {
                case CELL_CHKWALL:
                    res = isWallCell(bl, cell);
                    hookStop();
                    return res;
                case CELL_CHKPASS:
                case CELL_CHKREACH:
                    res = isWalkCell(bl, cell);
                    hookStop();
                    return res;
                case CELL_CHKNOPASS:
                case CELL_CHKNOREACH:
                    res = !isWalkCell(bl, cell);
                    hookStop();
                    return res;
                case CELL_GETTYPE:
                case CELL_CHKWATER:
                case CELL_CHKCLIFF:
                case CELL_CHKSTACK:
                case CELL_CHKNPC:
                case CELL_CHKBASILICA:
                case CELL_CHKLANDPROTECTOR:
                case CELL_CHKNOVENDING:
                case CELL_CHKNOCHAT:
                case CELL_CHKICEWALL:
                case CELL_CHKNOICEWALL:
                    break;
            }
        }
    }
    return 0;
}

struct mapcell emap_gat2cell(int *gatPtr)
{
    struct mapcell2 cell;
    const int gat = *gatPtr;

    memset(&cell, 0, sizeof(struct mapcell));

    switch (gat)
    {
        case 0: // walkable ground
            cell.walkable  = 1;
            cell.shootable = 1;
            cell.water     = 0;
            cell.air       = 0;
            cell.wall      = 0;
            break;
        case 1: // wall
            cell.walkable  = 0;
            cell.shootable = 0;
            cell.water     = 0;
            cell.air       = 0;
            cell.wall      = 1;
            break;
        case 2: // air allowed
            cell.walkable  = 0;
            cell.shootable = 0;
            cell.water     = 0;
            cell.air       = 1;
            cell.wall      = 0;
            break;
        case 3: // unwalkable water
            cell.walkable  = 0;
            cell.shootable = 1;
            cell.water     = 1;
            cell.air       = 0;
            cell.wall      = 0;
            break;
        case 4: // sit, walkable ground
            cell.walkable  = 1;
            cell.shootable = 1;
            cell.water     = 0;
            cell.air       = 0;
            cell.wall      = 0;
            break;
        default:
            ShowWarning("map_gat2cell: unrecognized gat type '%d'\n", gat);
            break;
    }

    hookStop();
    return strangeCast(struct mapcell, cell);
}

int emap_cell2gat(struct mapcell *cellPtr)
{
    struct mapcell2 cell = *((struct mapcell2*)cellPtr);
    hookStop();
    if (cell.walkable == 1 && cell.shootable == 1 && cell.water == 0)
        return 0;
    if (cell.walkable == 0 && cell.shootable == 0 && cell.water == 0 && cell.air == 0)
        return 1;
    if (cell.walkable == 0 && cell.shootable == 0 && cell.water == 0 && cell.air == 1)
        return 2;
    if (cell.walkable == 0 && cell.shootable == 1 && cell.water == 1)
        return 3;

    ShowWarning("map_cell2gat: cell has no matching gat type\n");
    return 1;
}

void emap_setgatcell2(int16 m, int16 x, int16 y, int gat)
{
    int j;

    if (m < 0 || m >= map->count ||
        x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys)
    {
        return;
    }

    j = x + y * map->list[m].xs;

    struct mapcell cell0 = map->gat2cell(gat);
    struct mapcell2 *cell = (struct mapcell2 *)&cell0;
    struct mapcell2 *cell2 = (struct mapcell2 *)&map->list[m].cell[j];
    cell2->walkable = cell->walkable;
    cell2->shootable = cell->shootable;
    cell2->water = cell->water;
    cell2->air = cell->air;
    cell2->wall = cell->wall;
}

void emap_setgatcell(int16 *mPtr, int16 *xPtr, int16 *yPtr, int *gatPtr)
{
    emap_setgatcell2(*mPtr, *xPtr, *yPtr, *gatPtr);
    hookStop();
}

bool emap_iwall_set(int16 *m, int16 *x, int16 *y, int *size, int8 *dir, bool *shootable, const char* wall_name)
{
    ShowError("Unsupported set wall function\n");
    hookStop();
    return false;
}

void emap_iwall_get(struct map_session_data *sd)
{
    if (!sd || map->list[sd->bl.m].iwall_num < 1)
    {
        hookStop();
        return;
    }

    DBIterator* iter = db_iterator(map->iwall_db);
    struct WallData *wall;
    for (wall = dbi_first(iter); dbi_exists(iter); wall = dbi_next(iter))
    {
        if (wall->m != sd->bl.m)
            continue;
        send_setwall_single(sd->fd, wall->m, wall->x1, wall->y1 , wall->x2 , wall->y2 , wall->mask);
    }
    dbi_destroy(iter);
    hookStop();
}

void emap_iwall_remove(const char *name)
{
    struct WallData *wall;

    if ((wall = (struct WallData *)strdb_get(map->iwall_db, name)) == NULL)
    {
        hookStop();
        return; // Nothing to do
    }

    int x;
    int y;
    int mask = 0;
    int x1 = wall->x1;
    int y1 = wall->y1;
    int x2 = wall->x2;
    int y2 = wall->y2;
    int m = wall->m;
    for (y = y1; y <= y2; y ++)
    {
        for (x = x1; x <= x2; x ++)
            emap_setgatcell2(m, x, y, mask); // default collision can be lost
    }

    send_setwall(m, x1, y1, x2, y2, mask, ALL_SAMEMAP);
    map->list[wall->m].iwall_num--;
    strdb_remove(map->iwall_db, wall->name);
    hookStop();
}

bool emap_iwall_set2(int m, int x1, int y1, int x2, int y2, int mask, const char *name)
{
    struct WallData *wall;

    if (!name)
        return false;

    if ((wall = (struct WallData *)strdb_get(map->iwall_db, name)) != NULL)
        return false; // Already Exists

    CREATE(wall, struct WallData, 1);
    wall->m = m;
    wall->x1 = x1;
    wall->y1 = y1;
    wall->x2 = x2;
    wall->y2 = y2;
    wall->mask = mask;
    safestrncpy(wall->name, name, sizeof(wall->name));

    int x;
    int y;
    for (y = y1; y <= y2; y ++)
    {
        for (x = x1; x <= x2; x ++)
            emap_setgatcell2(m, x, y, mask);
    }
    send_setwall(m, x1, y1, x2, y2, mask, ALL_SAMEMAP);

    strdb_put(map->iwall_db, wall->name, wall);
    map->list[m].iwall_num++;
    return true;
}
