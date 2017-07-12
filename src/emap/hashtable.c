// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2015 - 2016 Evol developers

#include "common/hercules.h"

#include "common/db.h"
#include "common/memmgr.h"
#include "emap/hashtable.h"

struct htreg_interface htreg_s;
struct htreg_interface *htreg;

int64 htreg_new_hashtable(void)
{
    int64 id = htreg->last_id++;
    struct DBMap *ht = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA,
                                   HT_MAX_KEY_LEN);
    i64db_put(htreg->htables, id, ht);
    return id;
}

bool htreg_destroy_hashtable(int64 id)
{
    struct DBMap *ht = i64db_get(htreg->htables, id);
    if (ht)
    {
        db_destroy(ht);
        i64db_remove(htreg->htables, id);
        return true;
    }
    return false;
}

bool htreg_hashtable_exists(int64 id)
{
    return i64db_exists(htreg->htables, id);
}

int64 htreg_hashtable_size(int64 id)
{
    struct DBMap *ht = i64db_get(htreg->htables, id);
    if (ht)
        return db_size(ht);
    return 0;
}

bool htreg_clear_hashtable(int64 id)
{
    struct DBMap *ht = i64db_get(htreg->htables, id);
    if (ht)
    {
        db_clear(ht);
        return true;
    }
    return false;
}

const struct DBData* htreg_hashtable_getvalue(int64 id, const char *key,
                                              const struct DBData *defval)
{
    struct DBMap *ht = i64db_get(htreg->htables, id);
    if (ht)
    {
        struct DBData *val = ht->get(ht, DB->str2key(key));
        return val ? val : defval;
    }
    return NULL;
}

bool htreg_hashtable_setvalue(int64 id, const char *key,
                              struct DBData value)
{
    struct DBMap *ht = i64db_get(htreg->htables, id);
    if (!ht)
        return false;

    bool keep = true;

    switch(value.type)
    {
        case DB_DATA_INT:
        case DB_DATA_UINT:
            keep = value.u.i;
            break;
        case DB_DATA_PTR:
            keep = value.u.ptr && *(char*)value.u.ptr;
            break;
    }

    if (keep)
        ht->put(ht, DB->str2key(key), value, NULL);
    else
        strdb_remove(ht, key);

    return true;
}

/**
 *   Iterators
 */
int64 htreg_create_iterator(int64 htId)
{
    struct DBMap *ht = i64db_get(htreg->htables, htId);
    if (ht)
    {
        int64 id = htreg->last_iterator_id++;
        struct DBIterator *it = db_iterator(ht);
        i64db_put(htreg->iterators, id, it);
        return id;
    }
    return 0;
}

bool htreg_destroy_iterator(int64 id)
{
    struct DBIterator *it = i64db_get(htreg->iterators, id);
    if (it)
    {
        dbi_destroy(it);
        i64db_remove(htreg->iterators, id);
        return true;
    }
    return false;
}

bool htreg_iterator_check(int64 id)
{
    struct DBIterator *it = i64db_get(htreg->iterators, id);
    return it ? dbi_exists(it) : false;
}

bool htreg_iterator_exists(int64 id)
{
    return i64db_exists(htreg->iterators, id);
}

const char* htreg_iterator_nextkey(int64 id)
{
    struct DBIterator *it = i64db_get(htreg->iterators, id);
    if (it)
    {
        union DBKey key;
        it->next(it, &key);
        if (dbi_exists(it))
            return key.str;
    }
    return NULL;
}

/**
 * Initializer.
 */
void htreg_init(void)
{
    htreg->htables = i64db_alloc(DB_OPT_BASE);
    htreg->iterators = i64db_alloc(DB_OPT_BASE);
}

/**
 * Finalizer.
 */
void htreg_final(void)
{

    if (htreg->htables != NULL)
    {
        struct DBMap *ht;
        struct DBIterator *iter = db_iterator(htreg->htables);
        for (ht = dbi_first(iter); dbi_exists(iter); ht = dbi_next(iter))
            db_destroy(ht);

        dbi_destroy(iter);
    }
    db_destroy(htreg->htables);

    // NOTE: maybe I should destroy iteratos before hashtables

    if (htreg->iterators != NULL)
    {
        struct DBIterator *it;
        struct DBIterator *iter = db_iterator(htreg->iterators);
        for (it = dbi_first(iter); dbi_exists(iter); it = dbi_next(iter))
            dbi_destroy(it);

        dbi_destroy(iter);
    }
    db_destroy(htreg->iterators);
}

/**
 * Interface defaults initializer.
 */
void htreg_defaults(void)
{
    htreg = &htreg_s;

    htreg->last_id = 1;
    htreg->htables = NULL;
    htreg->init = htreg_init;
    htreg->final = htreg_final;
    htreg->new_hashtable = htreg_new_hashtable;
    htreg->destroy_hashtable = htreg_destroy_hashtable;
    htreg->hashtable_exists = htreg_hashtable_exists;
    htreg->hashtable_size = htreg_hashtable_size;
    htreg->clear_hashtable = htreg_clear_hashtable;
    htreg->hashtable_getvalue = htreg_hashtable_getvalue;
    htreg->hashtable_setvalue = htreg_hashtable_setvalue;

    htreg->last_iterator_id = 1;
    htreg->iterators = NULL;
    htreg->create_iterator = htreg_create_iterator;
    htreg->destroy_iterator = htreg_destroy_iterator;
    htreg->iterator_check = htreg_iterator_check;
    htreg->iterator_exists = htreg_iterator_exists;
    htreg->iterator_nextkey = htreg_iterator_nextkey;
}
