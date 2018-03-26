// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2015 - 2016 Evol developers

#ifndef EVOL_HASH_TABLE
#define EVOL_HASH_TABLE

#include "common/hercules.h"
#include "common/db.h"

#define HT_MAX_KEY_LEN 32

struct htreg_interface
{
    int64 last_id;
    int64 last_iterator_id;
    struct DBMap *htables;
    struct DBMap *iterators;
    void (*init) (void);
    void (*final) (void);
    int64 (*new_hashtable) (void);
    bool (*destroy_hashtable) (int64 id);
    bool (*hashtable_exists) (int64 id);
    int64 (*hashtable_size) (int64 id);
    bool (*clear_hashtable) (int64 id);
    const struct DBData* (*hashtable_getvalue) (int64 id, const char *key,
                                                const struct DBData *defval);
    bool (*hashtable_setvalue) (int64 id, const char *key,
                                const struct DBData value);

    int64 (*create_iterator) (int64 id);
    bool (*destroy_iterator) (int64 id);
    bool (*iterator_check) (int64 id);
    bool (*iterator_exists) (int64 id);
    const char* (*iterator_firstkey) (int64 id);
    const char* (*iterator_lastkey) (int64 id);
    const char* (*iterator_nextkey) (int64 id);
    const char* (*iterator_prevkey) (int64 id);
};

void htreg_defaults(void);
void htreg_init(void);
void htreg_final(void);

HPShared struct htreg_interface *htreg;

#endif  // EVOL_HASH_TABLE
