// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/db.h"
#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/battle.h"
#include "map/itemdb.h"
#include "map/map.h"
#include "map/npc.h"
#include "map/pc.h"

#include "plugins/HPMHooking.h"

#include "emap/permission.h"
#include "emap/send.h"
#include "emap/data/itemd.h"
#include "emap/data/mapd.h"
#include "emap/data/mobd.h"
#include "emap/data/npcd.h"
#include "emap/data/session.h"
#include "emap/enum/beingflag.h"
#include "emap/struct/itemdext.h"
#include "emap/struct/mapdext.h"
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
        npc           : 1,
        basilica      : 1,
        landprotector : 1,
        novending     : 1,
        nochat        : 1,
        icewall       : 1,
        noicewall     : 1,

        wall        : 1,
        air         : 1,
        monsterWall : 1;

#ifdef CELL_NOSTACK
    int cell_bl; //Holds amount of bls in this cell.
#endif
};

int emap_addflooritem_post(int retVal,
                           const struct block_list *bl __attribute__ ((unused)),
                           struct item *item,
                           int amount __attribute__ ((unused)),
                           int16 m __attribute__ ((unused)),
                           int16 x __attribute__ ((unused)),
                           int16 y __attribute__ ((unused)),
                           int first_charid __attribute__ ((unused)),
                           int second_charid __attribute__ ((unused)),
                           int third_charid __attribute__ ((unused)),
                           int flags __attribute__ ((unused)),
                           bool showdropeffect __attribute__ ((unused)))
{
    TBL_ITEM* fitem = (TBL_ITEM*)idb_get(map->id_db, retVal);
    if (fitem)
    {
        if (fitem->cleartimer != INVALID_TIMER)
        {
            int timeout = battle->bc->flooritem_lifetime;
            struct ItemdExt *data = itemd_get_by_item(item);
            if (data)
                timeout = data->floorLifeTime;
            timer->delete(fitem->cleartimer, map->clearflooritem_timer);
            if (timeout >= 0)
                fitem->cleartimer = timer->add(timer->gettick() + timeout, map->clearflooritem_timer, fitem->bl.id, 0);
        }
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

    struct DBIterator* iter = db_iterator(map->pc_db);

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

        if (sd->status.sex == SEX_MALE)
            state = (state | BEINGFLAG_GENDER_MALE) & ~BEINGFLAG_GENDER_HIDDEN;
        else if (sd->status.sex == SEX_FEMALE)
            state &= ~(BEINGFLAG_GENDER_MALE | BEINGFLAG_GENDER_HIDDEN);
        else
            state = (state | BEINGFLAG_GENDER_HIDDEN) & ~BEINGFLAG_GENDER_MALE;

        if (pc_has_permission(sd, permission_send_gm_flag))
            state |= BEINGFLAG_GM;
        else
            state &= ~BEINGFLAG_GM;

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
    send_online_list(fd, buf, (unsigned int)(ptr - buf));
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
    if (!bl)
        return 0;
    if (bl->type == BL_NPC)
    {
        TBL_NPC *nd = (TBL_NPC *)bl;
        struct NpcdExt *ext = npcd_get(nd);
        if (ext)
            walkMask = ext->walkMask;
    }
    else if (bl->type == BL_MOB)
    {
        TBL_MOB *md = (TBL_MOB *)bl;
        struct MobdExt *ext = mobd_get_by_mob(md);
        if (ext)
            walkMask = ext->walkMask;
    }
    return walkMask;
}

static bool isWalkCell(const struct block_list *bl, struct mapcell2 cell)
{
    if (bl->type == BL_MOB)
    {
        if (cell.monsterWall)
            return false;
    }
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
    if (bl->type == BL_MOB)
    {
        if (cell.monsterWall)
            return true;
    }
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

int emap_getcellp_pre(struct map_data **mPtr,
                      const struct block_list **blPtr,
                      int16 *xPtr,
                      int16 *yPtr,
                      cell_chk *cellchkPtr)
{
    struct map_data *m = *mPtr;
    const struct block_list *bl = *blPtr;
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
                case CELL_CHKNOSKILL:
                    break;
            }
        }
    }
    return 0;
}

struct mapcell emap_gat2cell_pre(int *gatPtr)
{
    struct mapcell2 cell;
    const int gat = *gatPtr;

    memset(&cell, 0, sizeof(struct mapcell));

    switch (gat)
    {
        case 0: // walkable ground
        case 5: // should not happened
        case 4: // sit, walkable ground
            cell.walkable    = 1;
            cell.shootable   = 1;
            cell.water       = 0;
            cell.air         = 0;
            cell.wall        = 0;
            cell.monsterWall = 0;
            break;
        case 1: // wall
            cell.walkable    = 0;
            cell.shootable   = 0;
            cell.water       = 0;
            cell.air         = 0;
            cell.wall        = 1;
            cell.monsterWall = 0;
            break;
        case 2: // air allowed
            cell.walkable    = 0;
            cell.shootable   = 0;
            cell.water       = 0;
            cell.air         = 1;
            cell.wall        = 0;
            cell.monsterWall = 0;
            break;
        case 3: // unwalkable water
            cell.walkable    = 0;
            cell.shootable   = 1;
            cell.water       = 1;
            cell.air         = 0;
            cell.wall        = 0;
            cell.monsterWall = 0;
            break;
        case 6: // any monster cant walk, all other can
            cell.walkable    = 1;
            cell.shootable   = 1;
            cell.water       = 0;
            cell.air         = 0;
            cell.wall        = 0;
            cell.monsterWall = 1;
            break;
        default:
            ShowWarning("map_gat2cell: unrecognized gat type '%d'\n", gat);
            break;
    }

    hookStop();
    return strangeCast(struct mapcell, cell);
}

int emap_cell2gat_pre(struct mapcell *cellPtr)
{
    struct mapcell2 cell = *((struct mapcell2*)cellPtr);
    hookStop();
    if (cell.walkable == 1 && cell.shootable == 1 && cell.water == 0 && cell.monsterWall == 0)
        return 0;
    if (cell.walkable == 0 && cell.shootable == 0 && cell.water == 0 && cell.air == 0 && cell.monsterWall == 0)
        return 1;
    if (cell.walkable == 0 && cell.shootable == 0 && cell.water == 0 && cell.air == 1 && cell.monsterWall == 0)
        return 2;
    if (cell.walkable == 0 && cell.shootable == 1 && cell.water == 1 && cell.monsterWall == 0)
        return 3;
    if (cell.walkable == 1 && cell.shootable == 1 && cell.water == 0 && cell.monsterWall == 1)
        return 6;

    ShowWarning("map_cell2gat: cell has no matching gat type\n");
    return 1;
}

void emap_setgatcell2(int16 m,
                      int16 x,
                      int16 y,
                      int gat)
{
    int j;

    if (m < 0 ||
        m >= map->count ||
        x < 0 ||
        x >= map->list[m].xs ||
        y < 0 ||
        y >= map->list[m].ys)
    {
        return;
    }

    j = x + y * map->list[m].xs;

    if (map->list[m].cell == (struct mapcell *)0xdeadbeaf)
        map->cellfromcache(&map->list[m]);

    struct mapcell cell0 = map->gat2cell(gat);
    struct mapcell2 *cell = (struct mapcell2 *)&cell0;
    struct mapcell2 *cell2 = (struct mapcell2 *)&map->list[m].cell[j];
    cell2->walkable = cell->walkable;
    cell2->shootable = cell->shootable;
    cell2->water = cell->water;
    cell2->air = cell->air;
    cell2->wall = cell->wall;
}

void emap_setgatcell_pre(int16 *mPtr,
                         int16 *xPtr,
                         int16 *yPtr,
                         int *gatPtr)
{
    emap_setgatcell2(*mPtr, *xPtr, *yPtr, *gatPtr);
    hookStop();
}

bool emap_iwall_set_pre(int16 *m __attribute__ ((unused)),
                        int16 *x __attribute__ ((unused)),
                        int16 *y __attribute__ ((unused)),
                        int *size __attribute__ ((unused)),
                        int8 *dir __attribute__ ((unused)),
                        bool *shootable __attribute__ ((unused)),
                        const char **wall_namePtr __attribute__ ((unused)))
{
    ShowError("Unsupported set wall function\n");
    hookStop();
    return false;
}

void emap_iwall_get_pre(struct map_session_data **sdPtr)
{
    struct map_session_data *sd = *sdPtr;

    if (!sd || map->list[sd->bl.m].iwall_num < 1)
    {
        hookStop();
        return;
    }

    struct DBIterator* iter = db_iterator(map->iwall_db);
    struct WallData *wall;
    for (wall = dbi_first(iter); dbi_exists(iter); wall = dbi_next(iter))
    {
        if (wall->m != sd->bl.m)
            continue;
        send_setwall_single(sd->fd, wall->m, wall->layer, wall->x1, wall->y1 , wall->x2 , wall->y2 , wall->mask);
    }
    dbi_destroy(iter);
    hookStop();
}

bool emap_iwall_remove_pre(const char **namePtr)
{
    struct WallData *wall;
    const char *name = *namePtr;

    if ((wall = (struct WallData *)strdb_get(map->iwall_db, name)) == NULL)
    {
        hookStop();
        return false; // Nothing to do
    }

    int x;
    int y;
    int mask = 0;
    int x1 = wall->x1;
    int y1 = wall->y1;
    int x2 = wall->x2;
    int y2 = wall->y2;
    int m = wall->m;
    int layer = wall->layer;
    if (layer == 0)
    {
        for (y = y1; y <= y2; y ++)
        {
            for (x = x1; x <= x2; x ++)
                emap_setgatcell2(m, x, y, mask); // default collision can be lost
        }
    }

    send_setwall(m, layer, x1, y1, x2, y2, mask, ALL_SAMEMAP);
    map->list[wall->m].iwall_num--;
    strdb_remove(map->iwall_db, wall->name);
    hookStop();
    return true;
}

bool emap_iwall_set2(int m,
                     int layer,
                     int x1,
                     int y1,
                     int x2,
                     int y2,
                     int mask,
                     const char *name)
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
    wall->layer = layer;
    safestrncpy(wall->name, name, sizeof(wall->name));

    int x;
    int y;
    for (y = y1; y <= y2; y ++)
    {
        for (x = x1; x <= x2; x ++)
            emap_setgatcell2(m, x, y, mask);
    }
    send_setwall(m, layer, x1, y1, x2, y2, mask, ALL_SAMEMAP);

    strdb_put(map->iwall_db, wall->name, wall);
    map->list[m].iwall_num++;
    return true;
}

void map_alwaysVisible_add(const struct block_list *bl)
{
    if (!bl)
        return;
    struct MapdExt *data = mapd_get(bl->m);
    if (!data)
        return;
    int f;
    for (f = 0; f < VECTOR_LENGTH(data->npcs); f ++)
    {
        if (VECTOR_INDEX(data->npcs, f) == bl->id)
            return;
    }
    VECTOR_ENSURE(data->npcs, 1, 1);
    VECTOR_PUSH(data->npcs, bl->id);
}

bool map_alwaysVisible_find(const struct block_list *bl)
{
    if (!bl)
        return false;
    struct MapdExt *data = mapd_get(bl->m);
    if (!data)
        return false;
    int f;
    for (f = 0; f < VECTOR_LENGTH(data->npcs); f ++)
    {
        if (VECTOR_INDEX(data->npcs, f) == bl->id)
            return true;
    }
    return false;
}

void map_alwaysVisible_delete(const struct block_list *bl)
{
    if (!bl)
        return;
    struct MapdExt *data = mapd_get(bl->m);
    if (!data)
        return;
    int f;
    for (f = 0; f < VECTOR_LENGTH(data->npcs); f ++)
    {
        if (VECTOR_INDEX(data->npcs, f) == bl->id)
        {
            VECTOR_ERASE(data->npcs, f);
            return;
        }
    }
}

void map_alwaysVisible_send(TBL_PC *sd)
{
    if (!sd)
        return;
    int f;
    struct MapdExt *data = mapd_get(sd->bl.m);
    if (!data)
        return;

    for (f = 0; f < VECTOR_LENGTH(data->npcs); f ++)
    {
        const int id = VECTOR_INDEX(data->npcs, f);
        TBL_NPC *npc = map->id2nd(id);
        if (npc == NULL)
        {
            ShowError("npc present in always visible list "
                "but not in map: id=%d\n", id);
            continue;
        }
        clif->set_unit_idle(&npc->bl, sd, SELF);
        clif->blname_ack(sd->fd, &npc->bl);
    }
}

void map_clear_data(void)
{
    int f;
    for (f = 0; f < map->count; f++)
    {
        struct MapdExt *data = mapd_get(f);
        if (data)
            VECTOR_CLEAR(data->npcs);
    }
}

void edo_final_maps_pre(void)
{
    map_clear_data();
}
