// $Id: md5calc.h,v 1.1.1.1 2004/09/10 17:26:54 MagicalTux Exp $
#ifndef _MD5CALC_H_
#define _MD5CALC_H_
#include <netinet/in.h>

void MD5_String (const char *string, char *output);
void MD5_String2binary (const char *string, char *output);
char *MD5_saltcrypt(const char *key, const char *salt);
char *make_salt(void);
int pass_ok(const char *password, const char *crypted);
in_addr_t MD5_ip(char *secret, in_addr_t ip);

#endif
