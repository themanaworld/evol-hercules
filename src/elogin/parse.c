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
#include "common/timer.h"
#include "login/account.h"
#include "login/lclif.h"
#include "login/login.h"

#include "plugins/HPMHooking.h"

#include "elogin/config.h"
#include "elogin/md5calc.h"
#include "elogin/parse.h"
#include "elogin/send.h"

int clientVersion = 0;

void login_parse_version(int fd)
{
    struct login_session_data* sd = (struct login_session_data*)sockt->session[fd]->session_data;
    if (!sd)
        return;

    clientVersion = RFIFOL(fd, 2);

    if (clientVersion < 7)
    {
        lclif->login_error(fd, 5);
        return;
    }

    send_update_host(fd);
    send_server_version(fd);
}

bool elogin_client_login_pre(int *fdPtr,
                             struct login_session_data **sdPtr __attribute__ ((unused)))
{
    int fd = *fdPtr;
    uint16 command = RFIFOW(fd,0);
    if (command != 0x64)
    {
        lclif->login_error(fd, 3);
        hookStop();
        return true;
    }
    char username[NAME_LENGTH];
    safestrncpy(username, RFIFOP(fd, 6), NAME_LENGTH);
    int len = (int)safestrnlen(username, NAME_LENGTH);
    if (clientVersion < 7)
    {
        lclif->login_error(fd, 5);
        hookStop();
        return true;
    }
    else if (len >= 2 && username[len - 2] == '_' && memchr("FfMm", username[len - 1], 4))
    {
        lclif->login_error(fd, 3);
        hookStop();
        return true;
    }

    short *ptr = (short*)RFIFOP(fd, 2);
    if (*ptr == 20)
        *ptr = clientVersion;
    return false;
}

bool elogin_client_login_post(bool retVal,
                              int fd,
                              struct login_session_data* sd)
{
    sd = (struct login_session_data*)sockt->session[fd]->session_data;
    if (sd)
        sd->version = clientVersion;
    return retVal;
}

void elogin_parse_client_login2(int fd)
{
    char username[NAME_LENGTH];
    char password[PASSWD_LEN];
    char email[40];
    uint8 clienttype;
    int result;

    safestrncpy(username, RFIFOP(fd, 2), NAME_LENGTH);

    int len = (int)safestrnlen(username, NAME_LENGTH);
    if (len < 2 || !(username[len - 2] == '_') || !memchr("FfMm", username[len - 1], 4))
    {
        lclif->login_error(fd, 3);
        return;
    }

    safestrncpy(password, RFIFOP(fd, 26), NAME_LENGTH);
    safestrncpy(email, RFIFOP(fd, 51), 40);
    clienttype = RFIFOB(fd, 50);

    struct login_session_data* sd = (struct login_session_data*)sockt->session[fd]->session_data;
    if (!sd)
        return;

    char ip[16];
    uint32 ipl = sockt->session[fd]->client_addr;
    sockt->ip2str(ipl, ip);
    sd->clienttype = clienttype;
    sd->version = clientVersion;
    sd->passwdenc = 0;
    safestrncpy(sd->userid, username, NAME_LENGTH);
    ShowStatus("Request for connection of %s (ip: %s).\n", sd->userid, ip);
    safestrncpy(sd->passwd, password, PASSWD_LEN);

    if (e_mail_check(email) == 0)
    {
        ShowNotice("Attempt to create an e-mail REFUSED - e-mail is invalid (ip: %s)\n", ip);
        lclif->login_error(fd, 11);
        return;
    }

    result = login->mmo_auth(sd, false);

    if (result == -1)
    {
        int account_id = sd->account_id;
        struct mmo_account acc;
        if (!login->accounts->load_num(login->accounts, &acc, account_id))
        {
            ShowNotice("Attempt to create an e-mail on an account REFUSED - account: %d, ip: %s).\n", account_id, ip);
        }
        else
        {
            memcpy(acc.email, email, 40);
            ShowNotice("Create an e-mail on an account with a default e-mail (account: %d, new e-mail: %s, ip: %s).\n", account_id, email, ip);
            // Save
            login->accounts->save(login->accounts, &acc);
        }

        login->auth_ok(sd);
    }
    else
    {
        login->auth_failed(sd, result);
    }

    return;
}

enum parsefunc_rcode elogin_parse_ping_pre(int *fd __attribute__ ((unused)),
                                           struct login_session_data **sdPtr)
{
    struct login_session_data *sd = *sdPtr;
    if (!sd)
    {
        hookStop();
        return PACKET_VALID;
    }
    struct online_login_data* data = (struct online_login_data*)idb_get(login->online_db, sd->account_id);
    if (data == NULL)
    {
        hookStop();
        return PACKET_VALID;
    }
    if (data->waiting_disconnect != INVALID_TIMER)
    {
        timer->delete(data->waiting_disconnect, login->waiting_disconnect_timer);
        data->waiting_disconnect = timer->add(timer->gettick() + 30000, login->waiting_disconnect_timer, sd->account_id, 0);
    }
    hookStop();
    return PACKET_VALID;
}

void elogin_parse_change_paassword(int fd)
{
    char actual_pass[24], new_pass[24];
    int  status = 0;
    struct mmo_account acc;
    const int accountId = RFIFOL (fd, 2);

    memcpy (actual_pass, RFIFOP (fd, 6), 24);
    actual_pass[23] = '\0';
    memcpy (new_pass, RFIFOP (fd, 30), 24);
    new_pass[23] = '\0';

    if (!login->accounts->load_num(login->accounts, &acc, accountId))
    {
        // account not found
        send_char_password_change_ack(fd, accountId, 0);
        return;
    }

    if (!strcmp(actual_pass, acc.pass) || pass_ok(actual_pass, acc.pass))
    {
        // changed ok
        status = 1;
        safestrncpy(acc.pass, new_pass, sizeof(acc.pass));
        login->accounts->save(login->accounts, &acc);
    }
    else
    {
        // wrong password
        status = 2;
    }
    send_char_password_change_ack(fd, accountId, status);
}

void elogin_parse_serverexit(int fd)
{
    const int code = RFIFOW(fd, 2);
    switch (code)
    {
        case 100:  // all exit
        case 101:  // all restart
        case 104:  // git pull and all restart
        case 105:  // build all
        case 106:  // rebuild all
        case 107:  // git pull and build all
        case 108:  // git pull and rebuild all
        case 109:  // build plugin
        case 110:  // git pull and build plugin
            core->shutdown_callback();
            break;
        case 102:  // restart char and map server
        case 103:  // restart map server
            break;
        default:
            ShowWarning("Unknown termination code: %d\n", code);
    }
}
