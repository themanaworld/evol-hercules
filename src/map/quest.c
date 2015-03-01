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
#include "../../../map/itemdb.h"
#include "../../../map/quest.h"

#include "map/quest.h"

/**
 * Reads and parses an entry from the quest_db.
 *
 * @param cs     The config setting containing the entry.
 * @param n      The sequential index of the current config setting.
 * @param source The source configuration file.
 * @return The parsed quest entry.
 * @retval NULL in case of errors.
 */
struct quest_db *equest_read_db_sub(config_setting_t *cs, int *nPtr, const char *source)
{
    struct quest_db *entry = NULL;
    config_setting_t *t = NULL;
    int i32 = 0, quest_id;
    const char *str = NULL;
    const int n = *nPtr;

    /*
     * Id: Quest ID                    [int]
     * Name: Quest Name                [string]
     * TimeLimit: Time Limit (seconds) [int, optional]
     * Targets: (                      [array, optional]
     *     {
     *         MobId: Mob ID           [int]
     *         Count:                  [int]
     *     },
     *     ... (can repeated up to MAX_QUEST_OBJECTIVES times)
     * )
     * Drops: (
     *     {
     *         ItemId: Item ID to drop [int]
     *         Rate: Drop rate         [int]
     *         MobId: Mob ID to match  [int, optional]
     *     },
     *     ... (can be repeated)
     * )
     */

    if (!libconfig->setting_lookup_int(cs, "Id", &quest_id)) {
        ShowWarning("quest_read_db: Missing id in \"%s\", entry #%d, skipping.\n", source, n);
        hookStop();
        return NULL;
    }
    if (quest_id < 0 || quest_id >= MAX_QUEST_DB) {
        ShowWarning("quest_read_db: Invalid quest ID '%d' in \"%s\", entry #%d (min: 0, max: %d), skipping.\n", quest_id, source, n, MAX_QUEST_DB);
        hookStop();
        return NULL;
    }

    if (!libconfig->setting_lookup_string(cs, "Name", &str) || !*str) {
        ShowWarning("quest_read_db_sub: Missing Name in quest %d of \"%s\", skipping.\n", quest_id, source);
        hookStop();
        return NULL;
    }

    CREATE(entry, struct quest_db, 1);
    entry->id = quest_id;
    //safestrncpy(entry->name, str, sizeof(entry->name));

    if (libconfig->setting_lookup_int(cs, "TimeLimit", &i32)) // This is an unsigned value, do not check for >= 0
        entry->time = (unsigned int)i32;

    if ((t=libconfig->setting_get_member(cs, "Targets")) && config_setting_is_list(t)) {
/*
        int i, len = libconfig->setting_length(t);
        for (i = 0; i < len && entry->objectives_count < MAX_QUEST_OBJECTIVES; i++) {
            // Note: We ensure that objectives_count < MAX_QUEST_OBJECTIVES because
            //       quest_log (as well as the client) expect this maximum size.
            config_setting_t *tt = libconfig->setting_get_elem(t, i);
            int mob_id = 0, count = 0;
            if (!tt)
                break;
            if (!config_setting_is_group(tt))
                continue;
            if (!libconfig->setting_lookup_int(tt, "MobId", &mob_id) || mob_id <= 0)
                continue;
            if (!libconfig->setting_lookup_int(tt, "Count", &count) || count <= 0)
                continue;
            RECREATE(entry->objectives, struct quest_objective, ++entry->objectives_count);
            entry->objectives[entry->objectives_count-1].mob = mob_id;
            entry->objectives[entry->objectives_count-1].count = count;
        }
*/
        entry->objectives_count = 1;
        RECREATE(entry->objectives, struct quest_objective, 1);
        entry->objectives[0].mob = 1;
        entry->objectives[0].count = 0;
    }

    if ((t=libconfig->setting_get_member(cs, "Drops")) && config_setting_is_list(t)) {
        int i, len = libconfig->setting_length(t);
        for (i = 0; i < len; i++) {
            config_setting_t *tt = libconfig->setting_get_elem(t, i);
            int mob_id = 0, nameid = 0, rate = 0;
            if (!tt)
                break;
            if (!config_setting_is_group(tt))
                continue;
            if (!libconfig->setting_lookup_int(tt, "MobId", &mob_id))
                mob_id = 0; // Zero = any monster
            if (mob_id < 0)
                continue;
            if (!libconfig->setting_lookup_int(tt, "ItemId", &nameid) || !itemdb->exists(nameid))
                continue;
            if (!libconfig->setting_lookup_int(tt, "Rate", &rate) || rate <= 0)
                continue;
            RECREATE(entry->dropitem, struct quest_dropitem, ++entry->dropitem_count);
            entry->dropitem[entry->dropitem_count-1].mob_id = mob_id;
            entry->dropitem[entry->dropitem_count-1].nameid = nameid;
            entry->dropitem[entry->dropitem_count-1].rate = rate;
        }
    }
    hookStop();
    return entry;
}
