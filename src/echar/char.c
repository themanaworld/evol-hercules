// Copyright (c) Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// Copyright (c) 2014 - 2015 Evol developers

#include "common/hercules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/HPMi.h"
#include "common/memmgr.h"
#include "common/mapindex.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/sql.h"
#include "common/utils.h"
#include "common/timer.h"
#include "char/char.h"
#include "char/inter.h"

#include "plugins/HPMHooking.h"

#include "echar/char.h"
#include "echar/config.h"

void echar_parse_char_create_new_char(int *fdPtr, struct char_session_data **sdPtr)
{
    // ignore char creation disable option
    const int fd = *fdPtr;
    struct char_session_data *sd = *sdPtr;
    uint16 race = 0;
    uint16 look = 0;
    uint8 sex = 0;

    if (!sd)
        return;

    race = RFIFOW(fd, 31);
    if (race < min_char_class || race > max_char_class)
    {
        chr->creation_failed(fd, -10);
        RFIFOSKIP(fd, 31 + 5);
        hookStop();
        return;
    }
    sex = RFIFOB(fd, 33);
    if (sex != 0 && sex != 1 && sex != 3 && sex != 99)
    {
        chr->creation_failed(fd, -11);
        RFIFOSKIP(fd, 31 + 5);
        hookStop();
        return;
    }
    look = RFIFOW(fd, 34);
    if (look < min_look || look > max_look)
    {
        chr->creation_failed(fd, -12);
        RFIFOSKIP(fd, 31 + 5);
        hookStop();
        return;
    }

    // +++ need remove addition sql query after this line for set sex
    const int result = chr->make_new_char_sql(sd, RFIFOP(fd, 2), 1, 1, 1, 1, 1, 1, RFIFOB(fd, 26), RFIFOW(fd, 27), RFIFOW(fd, 29), JOB_NOVICE, 'U');
    if (result < 0)
    {
        chr->creation_failed(fd, result);
    }
    else
    {
        // retrieve data
        struct mmo_charstatus char_dat;
        chr->mmo_char_fromsql(result, &char_dat, false); //Only the short data is needed.

        char_dat.class = race;
        char_dat.sex = sex;
        char_dat.clothes_color = look;

        chr->mmo_char_tosql(result, &char_dat);
        char cSex = 'U';
        if (sex == SEX_MALE)
            cSex = 'M';
        else if (sex == SEX_FEMALE)
            cSex = 'F';

        if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `sex` = '%c' WHERE `char_id` = '%d'", "char", cSex, char_dat.char_id))
        {
            Sql_ShowDebug(inter->sql_handle);
        }
        chr->creation_ok(fd, &char_dat);

        // add new entry to the chars list
        sd->found_char[char_dat.slot] = result; // the char_id of the new char
    }
    RFIFOSKIP(fd, 31 + 5);
    hookStop();
}

static int tmpVersion = 0;

void echar_parse_char_connect_pre(int *fdPtr,
                                  struct char_session_data **sd __attribute__ ((unused)),
                                  uint32 *ipl __attribute__ ((unused)))
{
    tmpVersion = RFIFOW(*fdPtr, 14);
}

void echar_parse_char_connect_post(int fd,
                                   struct char_session_data *sd,
                                   uint32 ipl __attribute__ ((unused)))
{
    sd = (struct char_session_data*)sockt->session[fd]->session_data;
    if (sd)
        sd->version = tmpVersion;
}

void echar_creation_failed(int *fdPtr, int *result)
{
    const int fd = *fdPtr;
    WFIFOHEAD(fd, 3);
    WFIFOW(fd, 0) = 0x6e;
    /* Others I found [Ind] */
    /* 0x02 = Symbols in Character Names are forbidden */
    /* 0x03 = You are not eligible to open the Character Slot. */
    /* 0x0B = This service is only available for premium users.  */
    switch (*result)
    {
        case -1: WFIFOB(fd, 2) = 0x00; break; // 'Charname already exists'
        case -2: WFIFOB(fd, 2) = 0xFF; break; // 'Char creation denied'
        case -3: WFIFOB(fd, 2) = 0x01; break; // 'You are underaged'
        case -4: WFIFOB(fd, 2) = 0x03; break; // 'You are not eligible to open the Character Slot.'
        case -5: WFIFOB(fd, 2) = 0x02; break; // 'Symbols in Character Names are forbidden'
        case -10: WFIFOB(fd, 2) = 0x50; break; // Wrong class
        case -11: WFIFOB(fd, 2) = 0x51; break; // Wrong sex
        case -12: WFIFOB(fd, 2) = 0x52; break; // Wrong look

        default:
            ShowWarning("chr->parse_char: Unknown result received from chr->make_new_char_sql: %d!\n", *result);
            WFIFOB(fd,2) = 0xFF;
            break;
    }
    WFIFOSET(fd,3);
    hookStop();
}

void echar_parse_change_paassword(int fd)
{
    if (chr->login_fd < 0)
        return;
    struct char_session_data* sd = (struct char_session_data*)sockt->session[fd]->session_data;
    if (!sd)
        return;
    WFIFOHEAD(chr->login_fd, 54);
    WFIFOW(chr->login_fd, 0) = 0x5000;
    WFIFOL(chr->login_fd, 2) = sd->account_id;
    memcpy (WFIFOP (chr->login_fd, 6), RFIFOP (fd, 2), 24);
    memcpy (WFIFOP (chr->login_fd, 30), RFIFOP (fd, 26), 24);
    WFIFOSET(chr->login_fd, 54);
}

void echar_parse_login_password_change_ack(int charFd)
{
    struct char_session_data* sd = NULL;
    const int accountId = RFIFOL(charFd, 2);
    const int status = RFIFOB(charFd, 6);

    int fd = -1;
    ARR_FIND( 0, sockt->fd_max, fd, sockt->session[fd] && (sd = (struct char_session_data*)sockt->session[fd]->session_data) && sd->auth && sd->account_id == accountId );
    if (fd < sockt->fd_max && fd >= 0)
    {
        WFIFOHEAD(fd, 3);
        WFIFOW(fd, 0) = 0x62;
        WFIFOB(fd, 2) = status;
        WFIFOSET(fd, 3);
    }
}

void echar_mmo_char_send099d_post(int fd, struct char_session_data *sd)
{
    send_additional_slots(fd, sd);
}

int echar_mmo_char_send_characters_post(int retVal,
                                        int fd,
                                        struct char_session_data* sd)
{
    send_additional_slots(fd, sd);
    return retVal;
}

void send_additional_slots(int fd, struct char_session_data* sd)
{
    int char_id;
    int name_id;
    int slot;
    short card0;
    short card1;
    short card2;
    short card3;

    if (!sd)
        return;

    struct SqlStmt* stmt = SQL->StmtMalloc(inter->sql_handle);
    if (stmt == NULL)
    {
        SqlStmt_ShowDebug(stmt);
        return;
    }

    if (SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT "
        "inventory.char_id, inventory.nameid, inventory.equip, "
        "inventory.card0, inventory.card1, inventory.card2, inventory.card3 "
        "FROM `char` "
        "LEFT JOIN inventory ON inventory.char_id = `char`.char_id "
        "WHERE account_id = '%d' AND equip <> 0 AND amount > 0 ORDER BY inventory.char_id", sd->account_id)
        || SQL_ERROR == SQL->StmtExecute(stmt)
        || SQL_ERROR == SQL->StmtBindColumn(stmt, 0,  SQLDT_INT,   &char_id, 0, NULL, NULL)
        || SQL_ERROR == SQL->StmtBindColumn(stmt, 1,  SQLDT_INT,   &name_id, 0, NULL, NULL)
        || SQL_ERROR == SQL->StmtBindColumn(stmt, 2,  SQLDT_INT,   &slot, 0, NULL, NULL)
        || SQL_ERROR == SQL->StmtBindColumn(stmt, 3,  SQLDT_SHORT, &card0, 0, NULL, NULL)
        || SQL_ERROR == SQL->StmtBindColumn(stmt, 4,  SQLDT_SHORT, &card1, 0, NULL, NULL)
        || SQL_ERROR == SQL->StmtBindColumn(stmt, 5,  SQLDT_SHORT, &card2, 0, NULL, NULL)
        || SQL_ERROR == SQL->StmtBindColumn(stmt, 6,  SQLDT_SHORT, &card3, 0, NULL, NULL))
    {
        SqlStmt_ShowDebug(stmt);
        SQL->StmtFree(stmt);
        return;
    }

    while (SQL_SUCCESS == SQL->StmtNextRow(stmt))
    {
        int type = 2;
        switch (slot)
        {
            case 0:
                type = 0;
                break;
            case EQP_HEAD_LOW:
                type = 3;
                break;
            case EQP_HAND_R:
                type = 2;
                break;
            case EQP_GARMENT:
                type = 12;
                break;
            case EQP_ACC_L:
                type = 19;
                break;
            case EQP_ARMOR:
                type = 17;
                break;
            case EQP_HAND_L:
                type = 8;
                break;
            case EQP_SHOES:
                type = 9;
                break;
            case EQP_ACC_R:
                type = 18;
                break;
            case EQP_HEAD_TOP:
                type = 4;
                break;
            case EQP_HEAD_MID:
                type = 5;
                break;
            case EQP_COSTUME_HEAD_TOP:
                type = 13;
                break;
            case EQP_COSTUME_HEAD_MID:
                type = 14;
                break;
            case EQP_COSTUME_HEAD_LOW:
                type = 15;
                break;
            case EQP_COSTUME_GARMENT:
                type = 16;
                break;
            dafault:
                ShowWarning("unknown equip for char %d, item %d, slot %d, cards %d,%d,%d,%d\n", char_id, name_id, slot, (int)card0, (int)card1, (int)card2, (int)card3);
                type = 0;
                break;
        }

        if (type == 0)
            continue;
        WFIFOHEAD (fd, 19);
        WFIFOW (fd, 0) = 0xb17;
        WFIFOL (fd, 2) = char_id;
        WFIFOB (fd, 6) = (unsigned char)type;
        WFIFOW (fd, 7) = name_id;
        WFIFOW (fd, 9) = 0;
        WFIFOW (fd, 11) = card0;
        WFIFOW (fd, 13) = card1;
        WFIFOW (fd, 15) = card2;
        WFIFOW (fd, 17) = card3;
        WFIFOSET (fd, 19);

        //ShowWarning("char %d, item %d, slot %d->%d, cards %d,%d,%d,%d\n", char_id, name_id, slot, type, (int)card0, (int)card1, (int)card2, (int)card3);
    }

    SQL->StmtFree(stmt);
}

void echar_parse_frommap_request_stats_report_pre(int *fdPtr)
{
    const int fd = *fdPtr;
    RFIFOSKIP(fd, 2);  /* we skip first 2 bytes which are the 0x3008, so we end up with a buffer equal to the one we send */
    RFIFOSKIP(fd, RFIFOW(fd,2));  /* skip this packet */
    RFIFOFLUSH(fd);
    hookStop();
}

void echar_parse_map_serverexit(int mapFd)
{
    const int code = RFIFOW(mapFd, 2);
    switch (code)
    {
        case 100:  // all exit
        case 101:  // all restart
        case 102:  // restart char and map server
        case 104:  // git pull and restart all restart
        case 105:  // build all
        case 106:  // rebuild all
        case 107:  // git pull and build all
        case 108:  // git pull and rebuild all
        case 109:  // build plugin
        case 110:  // git pull and build plugin
            echat_send_login_serverexit(code);
            HSleep(1);
            core->shutdown_callback();
            break;
        case 103:  // restart map server
            break;
        default:
            ShowWarning("Unknown termination code: %d\n", code);
    }
}

void echat_send_login_serverexit(const int code)
{
    WFIFOHEAD(chr->login_fd, 4);
    WFIFOW(chr->login_fd, 0) = 0x5003;
    WFIFOW(chr->login_fd, 2) = code;
    WFIFOSET(chr->login_fd, 4);
}
