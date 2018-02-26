// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/script.h"

#include "map/chrif.h"
#include "map/clif.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/script.h"
#include "map/quest.h"

#include "emap/utils/formatutils.h"
#include "emap/lang.h"

int format_sub(struct script_state* st, int translate)
{
    if (!st)
        return 0;

    TBL_PC *sd = NULL;
    if (translate)
        sd = script->rid2sd(st);

    if (!script_hasdata(st, 3))
    {
        if (sd)
        {
            if (translate == 2)
            {
                char *buf = aCalloc (strlen(script_getstr(st, 2)) + 3, sizeof(char));
                strcpy (buf, script_getstr(st, 2));
                if (sd->status.sex)
                    strcat (buf, "#1");
                else
                    strcat (buf, "#0");
                script_pushstr(st, aStrdup(lang_pctrans(buf, sd)));
                aFree (buf);
            }
            else
            {
                script_pushstr(st, aStrdup(lang_pctrans(script_getstr(st, 2), sd)));
            }
        }
        else
        {
            script_pushstr(st, aStrdup(script_getstr(st, 2)));
        }
        return 1;
    }

    char *line = (char *) aCalloc (550, sizeof (char));
    int idx = 3;
    if (sd)
    {
        if (translate == 2)
        {
            const char *str = NULL;
            char *buf = NULL;
            if (sd->status.sex)
            {
                str = script_getstr(st, 3);
                buf = aCalloc (strlen(str) + 3, sizeof(char));
                strcpy (buf, str);
                strcat (buf, "#1");
            }
            else
            {
                str = script_getstr(st, 2);
                buf = aCalloc (strlen(str) + 3, sizeof(char));
                strcpy (buf, str);
                strcat (buf, "#0");
            }
            strcpy(line, lang_pctrans(buf, sd));
            aFree (buf);
            idx = 4;
        }
        else
        {
            strcpy(line, lang_pctrans(script_getstr(st, 2), sd));
        }
    }
    else
    {
        strcpy(line, script_getstr(st, 2));
    }

    char *ptr = line;
    int sz = (int)strlen(line);
    while (script_hasdata(st, idx))
    {
        char *tmp = strstr(ptr, "@@");
        if (!tmp)
            break;
        const char *item = script_getstr(st, idx);
        int len = (int)strlen(item);
        if (len > 50)
            break;
        sz += len - 2;
        if (sz > 490)
            break;
        memmove(tmp + len, tmp + 2, strlen(tmp + 2) + 1);
        memcpy(tmp, item, len);
        ptr = tmp + len;
        idx ++;
    }

    script_pushstr(st, line);
    return 0;
}
