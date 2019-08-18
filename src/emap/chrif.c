// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2019 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/nullpo.h"
#include "common/socket.h"
#include "map/chrif.h"
#include "map/pc.h"

#include "emap/chrif.h"

#include "plugins/HPMHooking.h"

/**
 * save sex change
 */
bool echrif_changesex(TBL_PC **sdPtr, bool *change_account __attribute__ ((unused)))
{
    const TBL_PC *sd = *sdPtr;
	nullpo_retr(false, sd);

	if (!chrif->isconnected())
        return false;

	WFIFOHEAD(chrif->fd, 44);
	WFIFOW(chrif->fd, 0) = 0x2b0e;
	WFIFOL(chrif->fd, 2) = sd->status.account_id;
	safestrncpy(WFIFOP(chrif->fd, 6), sd->status.name, NAME_LENGTH);
	WFIFOW(chrif->fd, 30) = CHAR_ASK_NAME_CHANGECHARSEX;
	WFIFOB(chrif->fd, 32) = sd->status.sex;
	WFIFOSET(chrif->fd, 44);

    hookStop();
	return true;
}
