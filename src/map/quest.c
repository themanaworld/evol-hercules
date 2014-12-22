// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 Evol developers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../common/HPMi.h"
#include "../../../common/malloc.h"
#include "../../../common/mmo.h"
#include "../../../common/socket.h"
#include "../../../common/strlib.h"
#include "../../../map/quest.h"

#include "map/quest.h"

int equest_read_db(void)
{
    FILE *fp;
    char line[1024];
    int i, count = 0;
    char *str[20], *p, *np;
    struct quest_db entry;

    sprintf(line, "%s/quest_db.txt", map->db_path);
    if ((fp = fopen(line, "r")) == NULL)
    {
        ShowError("can't read %s\n", line);
        hookStop();
        return -1;
    }

    while (fgets(line, sizeof(line), fp))
    {
        if (line[0]=='/' && line[1]=='/')
            continue;
        memset(str,0,sizeof(str));

        for (i = 0, p = line; i < 8; i++) {
            if (( np = strchr(p,',') ) != NULL) {
                str[i] = p;
                *np = 0;
                p = np + 1;
            } else if (str[0] == NULL) {
                break;
            } else {
                ShowError("quest_read_db: insufficient columns in line %s\n", line);
                continue;
            }
        }
        if (str[0] == NULL)
            continue;

        memset(&entry, 0, sizeof(entry));

        entry.id = atoi(str[0]);

        if (entry.id < 0 || entry.id >= MAX_QUEST_DB) {
            ShowError("quest_read_db: Invalid quest ID '%d' in line '%s' (min: 0, max: %d.)\n", entry.id, line, MAX_QUEST_DB);
            continue;
        }

        entry.time = atoi(str[1]);

//        for (i = 0; i < MAX_QUEST_OBJECTIVES; i++) {
        for (i = 0; i < 1; i++) {
//            entry.mob[i] = atoi(str[2*i+2]);
            entry.mob[i] = 1;
            entry.count[i] = atoi(str[2*i+3]);
        }

        entry.num_objectives = i;

        if (quest->db_data[entry.id] == NULL)
            quest->db_data[entry.id] = aMalloc(sizeof(struct quest_db));

        memcpy(quest->db_data[entry.id], &entry, sizeof(struct quest_db));
        count++;
    }
    fclose(fp);
    ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, "quest_db.txt");
    hookStop();
    return 0;
}
