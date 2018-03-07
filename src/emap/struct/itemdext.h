// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

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
    int requiredSkill;

    int useEffect;
    int useFailEffect;
    int unequipEffect;
    int unequipFailEffect;

    int minRange;

    int tmpUseType;

    VECTOR_DECL(int) allowedAmmo;
    VECTOR_DECL(struct ItemCardExt) allowedCards;
    struct script_code *dropScript;
    struct script_code *takeScript;
    struct script_code *insertScript;

    int16_t subX;
    int16_t subY;
    bool allowPickup;
    bool charmItem;
    bool identified;
};

#endif  // EVOL_MAP_ITEMDEXT
