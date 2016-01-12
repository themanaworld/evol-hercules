// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#ifndef EVOL_MAP_SCRIPT
#define EVOL_MAP_SCRIPT

void eset_reg_npcscope_num(struct script_state* st, struct reg_db *n, int64 *num, const char* name, int *val);
int eget_val_npcscope_num(struct script_state* st, struct reg_db *n, struct script_data* data);
void eset_reg_npcscope_str(struct script_state* st, struct reg_db *n, int64 *num, const char* name, const char *str);
char *eget_val_npcscope_str(struct script_state* st, struct reg_db *n, struct script_data* data);
void script_run_item_amount_script(TBL_PC *sd, struct script_code *itemScript, int itemId, int amount);
void script_run_card_script(TBL_PC *sd, struct script_code *itemScript, int itemId, int cardId);

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
BUILDIN(requestCraft);
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
BUILDIN(setBgTeam);
BUILDIN(chatJoin);
BUILDIN(checkNpcCell);
BUILDIN(setCells);
BUILDIN(delCells);
BUILDIN(setSkin);
BUILDIN(initCraft);
BUILDIN(dumpCraft);
BUILDIN(deleteCraft);
BUILDIN(getCraftSlotId);
BUILDIN(getCraftSlotAmount);

#endif  // EVOL_MAP_SCRIPT
