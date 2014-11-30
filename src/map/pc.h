// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_PC
#define EVOL_MAP_PC

enum VarConst
{
    Const_ClientVersion = 10000
};

int epc_readparam_pre(struct map_session_data* sd, int *type);

int epc_setregistry(struct map_session_data *sd, int64 *reg, int *val);

void epc_equipitem_pos(struct map_session_data *sd, struct item_data *id, int *posPtr);

void epc_unequipitem_pos(struct map_session_data *sd, int *nPtr, int *posPtr);

bool epc_can_attack (struct map_session_data *sd, int *target_id);

#endif  // EVOL_MAP_PC
