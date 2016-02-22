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
#include "map/npc.h"
#include "map/pc.h"

#include "emap/config.h"
#include "emap/lang.h"
#include "emap/data/session.h"
#include "emap/struct/sessionext.h"

#define MAX_LANGS 100

struct DBMap *translate_db = NULL;
char *lang_langs[MAX_LANGS];
int lang_num = 0;

static int langsdb_readdb (void);
static int langsdb_readlangs (void);

void do_init_langs (void)
{
    translate_db = strdb_alloc(DB_OPT_RELEASE_BOTH, 1000);

    langsdb_readlangs ();
    langsdb_readdb ();
}

static int delete_lang_sub(DBKey key __attribute__ ((unused)),
                           DBData *data,
                           va_list args __attribute__ ((unused)))
{
    int f;
    char **strings = DB->data2ptr(data);
    for (f = 0; f < lang_num; f ++)
    {
        aFree(strings[f]);
        strings[f] = NULL;
    }
    return 0;
}

void do_final_langs(void)
{
    translate_db->destroy(translate_db, delete_lang_sub);
    int f;
    for (f = 0; f < lang_num; f ++)
        aFree(lang_langs[f]);
}

static int langsdb_readlangs (void)
{
    FILE *fp;
    lang_num = 0;
    fp = fopen("langs/langs.txt", "r");
    if (fp == NULL)
    {
        printf ("can't read langs/langs.txt\n");
        return 1;
    }
    char line[100];
    char text[101];
    while (fgets (line, 99, fp))
    {
        if (sscanf(line, "%99s\n", text) < 1)
            continue;

        lang_langs[lang_num] = aStrdup (text);
        lang_num ++;
    }
    fclose(fp);
    return 0;
}

static int langsdb_readdb (void)
{
    char line[1020], line1[1020], line2[1020];
    char filename[1000];
    char **strings = NULL;
    char *idx;
    int i;
    int sz;
    for (i = 0; i < lang_num; i ++)
    {
        strcpy (filename, "langs/lang_");
        strcat (filename, lang_langs[i]);
        strcat (filename, ".txt");

        FILE *fp = fopen(filename, "r");
        if (fp == NULL)
        {
            printf ("can't read %s\n", filename);
            return 1;
        }

        if (!fgets (line, 1010, fp))
        {
            printf ("can't read %s\n", filename);
            continue;
        }

        line1[0] = 0;
        line2[0] = 0;
        while (fgets (line, 1010, fp))
        {
            if (*line)
            {
                idx = strrchr (line, '\n');
                if (idx)
                    *idx = 0;
            }

            if (!*line)
            {
                line1[0] = 0;
                line2[0] = 0;
                continue;
            }
            else if (!*line1)
            {
                strcpy (line1, line);
                continue;
            }
            strcpy (line2, line);

            strings = (char **)strdb_get(translate_db, line1);
            if (!strings)
            {
                strings = aCalloc (lang_num, sizeof(int*));
                sz = strlen(line1) + 1;
                strings[0] = aCalloc (sz < 24 ? 24 : sz, sizeof(char));
                strcpy (strings[0], line2);
                strdb_put(translate_db, aStrdup (line1), strings);
            }
            else
            {
                sz = strlen(line2) + 1;
                strings[i] = aCalloc (sz < 24 ? 24 : sz, sizeof(char));
                strcpy (strings[i], line2);
            }

            *line1 = 0;
            *line2 = 0;
        }
        fclose (fp);
    }
    return 0;
}

const char* lang_trans(const char *str, int lng, int flg)
{
    if (!str)
        return 0;

    if (lng < 0 || lng >= lang_num || !translate_db || !translate_db->get)
        return str;

    char **strings = (char **)strdb_get(translate_db, str);
    if (!strings)
    {
        if (flg)
            printf ("no translations for: %s\n", str);
        return str;
    }

    if (!strings[lng])
    {
        if (warn_missing_translation && flg)
            printf ("no lang string (%s) for: %s\n", lang_langs[lng], str);
        return str;
    }

    return strings[lng];
}

const char* lang_pctrans(const char *str, TBL_PC *sd)
{
    int lng = 0;
    int flg = 1;
    if (!str)
        return 0;

    if (*str == '#')
        flg = 0;
    if (sd)
    {
        struct SessionExt *data = session_get_bysd(sd);
        if (data)
            lng = data->language;
    }

    return lang_trans(str, lng, flg);
}

int lang_getId(const char *str)
{
    char *str1 = aStrdup(str);
    char *str2 = NULL;
    int f;

    if ((str2 = strchr(str, '.')))
        *str2 = 0;

    for (f = 0; f < MAX_LANGS && lang_langs[f]; f ++)
    {
        if (!strcmp(str, lang_langs[f]))
        {
            aFree (str1);
            return f;
        }
    }

    if ((str2 = strchr(str1, '_')))
        *str2 = 0;

    for (f = 0; f < MAX_LANGS && lang_langs[f]; f ++)
    {
        if (strstr(lang_langs[f], str1) == lang_langs[f])
        {
            aFree (str1);
            return f;
        }
    }
    aFree (str1);
    return -1;
}
