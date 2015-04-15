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
#include "../../../common/timer.h"
#include "../../../login/account.h"
#include "../../../login/login.h"

#include "common/ip.h"
#include "login/config.h"
#include "login/md5calc.h"
#include "login/parse.h"
#include "login/send.h"

int clientVersion = 0;

void login_parse_version(int fd)
{
    struct login_session_data* sd = (struct login_session_data*)session[fd]->session_data;
    if (!sd)
        return;

    clientVersion = RFIFOL(fd, 2);

    if (clientVersion < 2)
    {
        login->login_error(fd, 5);
        return;
    }

    send_update_host(fd);
    send_server_version(fd);
}

int elogin_parse_client_login_pre(int *fdPtr,
                                  struct login_session_data* sd __attribute__ ((unused)),
                                  const char *const ip __attribute__ ((unused)))
{
    int fd = *fdPtr;
    uint16 command = RFIFOW(fd,0);
    if (command != 0x64)
    {
        login->login_error(fd, 3);
        hookStop();
        return 1;
    }
    char username[NAME_LENGTH];
    safestrncpy(username, (const char*)RFIFOP(fd, 6), NAME_LENGTH);
    int len = strnlen(username, NAME_LENGTH);
    if (clientVersion < 2)
    {
        login->login_error(fd, 5);
        hookStop();
        return 1;
    }
    else if (len >= 2 && username[len - 2] == '_' && memchr("FfMm", username[len - 1], 4))
    {
        login->login_error(fd, 3);
        hookStop();
        return 1;
    }

    return 0;
}

void elogin_parse_client_login2(int fd)
{
    char username[NAME_LENGTH];
    char password[PASSWD_LEN];
    char email[40];
    uint8 clienttype;
    int result;

    safestrncpy(username, (const char*)RFIFOP(fd, 2), NAME_LENGTH);

    int len = strnlen(username, NAME_LENGTH);
    if (len < 2 || !username[len - 2] == '_' || !memchr("FfMm", username[len - 1], 4))
    {
        login->login_error(fd, 3);
        return;
    }

    safestrncpy(password, (const char*)RFIFOP(fd, 26), NAME_LENGTH);
    safestrncpy(email, (const char*)RFIFOP(fd, 51), 40);
    clienttype = RFIFOB(fd, 50);

    struct login_session_data* sd = (struct login_session_data*)session[fd]->session_data;
    if (!sd)
        return;

    char ip[16];
    uint32 ipl = session[fd]->client_addr;
    ip2str(ipl, ip);
    sd->clienttype = clienttype;
    sd->version = clientVersion;
    sd->passwdenc = 0;
    safestrncpy(sd->userid, username, NAME_LENGTH);
    ShowStatus("Request for connection of %s (ip: %s).\n", sd->userid, ip);
    safestrncpy(sd->passwd, password, PASSWD_LEN);

    if (e_mail_check(email) == 0)
    {
        ShowNotice("Attempt to create an e-mail REFUSED - e-mail is invalid (ip: %s)\n", ip);
        login->login_error(fd, 11);
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

void elogin_parse_request_connection(int *fd, struct login_session_data* sd, const char *const ip)
{
    if (!inter_server_ip || !ip)
        return;
    if (!checkAllowedIp(inter_server_ip, ip))
    {
        hookStop();
        login->char_server_connection_status(*fd, sd, 3);
        ShowNotice("Connection of the char-server from ip %s REFUSED.\n", ip);
    }
}

void elogin_parse_ping(int *fd, struct login_session_data* sd)
{
    RFIFOSKIP(*fd, 26);
    if (!sd)
    {
        hookStop();
        return;
    }
    struct online_login_data* data = (struct online_login_data*)idb_get(login->online_db, sd->account_id);
    if (data == NULL)
    {
        hookStop();
        return;
    }
    if (data->waiting_disconnect != INVALID_TIMER)
    {
        timer->delete(data->waiting_disconnect, login->waiting_disconnect_timer);
        data->waiting_disconnect = timer->add(timer->gettick() + 30000, login->waiting_disconnect_timer, sd->account_id, 0);
    }
    hookStop();
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
