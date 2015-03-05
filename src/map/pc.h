// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_PC
#define EVOL_MAP_PC

enum VarConst
{
    Const_ClientVersion = 10000
};

int epc_readparam_pre(TBL_PC* sd, int *type);

int epc_setregistry(TBL_PC *sd, int64 *reg, int *val);

void epc_equipitem_pos(TBL_PC *sd, struct item_data *id, int *posPtr);

void epc_unequipitem_pos(TBL_PC *sd, int *nPtr, int *posPtr);

bool epc_can_attack (TBL_PC *sd, int *target_id);

int epc_takeitem(TBL_PC *sd, struct flooritem_data *fitem);

void epc_validate_levels(void);

#endif  // EVOL_MAP_PC
