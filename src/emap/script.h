// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#ifndef EVOL_MAP_SCRIPT
#define EVOL_MAP_SCRIPT

void escript_set_reg_npc_num(struct script_state* st, struct reg_db *n, int64 *num, const char* name, int *val);
int escript_get_val_npcscope_num(struct script_state* st, struct reg_db *n, struct script_data* data);

BUILDIN(l);
BUILDIN(lg);
BUILDIN(setCamNpc);
BUILDIN(setCam);
BUILDIN(moveCam);
BUILDIN(restoreCam);
BUILDIN(npcTalk3);
BUILDIN(closeDialog);
BUILDIN(shop);
BUILDIN(getItemLink);
BUILDIN(requestLang);
BUILDIN(requestItem);
BUILDIN(requestItems);
BUILDIN(requestItemIndex);
BUILDIN(requestItemsIndex);
BUILDIN(getq);
BUILDIN(setq);
BUILDIN(setNpcDir);
BUILDIN(rif);
BUILDIN(countItemColor);
BUILDIN(miscEffect);
BUILDIN(setMapMask);
BUILDIN(getMapMask);
BUILDIN(addMapMask);
BUILDIN(removeMapMask);
BUILDIN(setNpcSex);
BUILDIN(showAvatar);
BUILDIN(setAvatarDir);
BUILDIN(setAvatarAction);
BUILDIN(clear);
BUILDIN(changeMusic);
BUILDIN(setNpcDialogTitle);
BUILDIN(getMapName);
BUILDIN(unequipById);
BUILDIN(isPcDead);
BUILDIN(areaTimer);
BUILDIN(getAreaDropItem);
BUILDIN(setMount);
BUILDIN(clientCommand);
BUILDIN(isUnitWalking);
BUILDIN(failedRefIndex);
BUILDIN(downRefIndex);
BUILDIN(successRefIndex);
BUILDIN(isStr);
BUILDIN(npcSit);
BUILDIN(npcStand);
BUILDIN(npcWalkTo);

#endif  // EVOL_MAP_SCRIPT
