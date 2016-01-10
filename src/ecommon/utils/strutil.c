// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/memmgr.h"
#include "common/strlib.h"

#include "ecommon/utils/strutil.h"

#include "ecommon/struct/strutildata.h"

struct strutil_data *strutil_split(const char *str,
                                   const char separator,
                                   const int len)
{
    if (!str || len < 1)
        return NULL;
    struct strutil_data *data = aCalloc(1, sizeof(struct strutil_data));
    if (!data)
        return NULL;
    data->str = aStrdup(str);
    VECTOR_INIT(data->parts);
    VECTOR_ENSURE(data->parts, len + 1, 1);

    data->len = sv->split(data->str,
        strlen(data->str),
        0,
        separator,
        VECTOR_DATA(data->parts),
        len + 1,
        0);
    if (data->len < 1)
    {
        strutil_free(data);
        return NULL;
    }

    return data;
}

void strutil_free(struct strutil_data *data)
{
    if (!data)
        return NULL;
    VECTOR_CLEAR(data->parts);
    aFree(data->str);
    aFree(data);
}
