// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_ITEMDEXT
#define EVOL_MAP_ITEMDEXT

struct ItemCardExt
{
    unsigned short id;
    unsigned short amount;
};

struct ItemdExt
{
    int floorLifeTime;

    int requiredStr;
    int requiredAgi;
    int requiredVit;
    int requiredInt;
    int requiredDex;
    int requiredLuk;
    int requiredMaxHp;
    int requiredMaxSp;
    int requiredAtk;
    int requiredMAtkMin;
    int requiredMAtkMax;
    int requiredDef;
    int requiredMDef;

    int useEffect;
    int useFailEffect;
    int unequipEffect;
    int unequipFailEffect;

    struct ItemCardExt allowedCards[100];

    bool allowPickup;
    bool charmItem;
};

#endif  // EVOL_MAP_ITEMDEXT
