/*
 * Copyright (c) 1998, 1999 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <roken.h>

#include <sys/types.h>
#include <rx/rx.h>
#include <rx/rx_null.h>

#include <assert.h>

#ifdef KERBEROS
#include <openssl/des.h>
#include <krb5.h>
#include <rxkad.h>
#include <rxkad_locl.h>
#endif

#include <err.h>
#include <ctype.h>

#ifndef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <service.h>

#include "rx/rxgencon.h"

#include "msecurity.h"
#include "acl.h"

RCSID("$Id: msecurity.c,v 1.13 2005/01/17 09:09:33 lha Exp $");

#if defined(KERBEROS) && defined(HAVE_KRB4)
static char acl_file[] = MILKO_SYSCONFDIR "/superuserlist"; /* XXX */
#endif

/*
 * Don't test for superuser if set
 */

static int superuser_check_disabled = FALSE;

void
sec_disable_superuser_check (void)
{
    superuser_check_disabled = TRUE;
}

/*
 * Return 0 if the user is not a superuser
 *        1 if the user is a superuser
 */

int
sec_is_superuser(struct rx_call *call)
{
    char *name;

    if (superuser_check_disabled)
	return 1;

    if (sec_getname(call->conn, &name))
	return 0;
#if defined(KERBEROS) && defined(HAVE_KRB4)
    if (call->conn->securityIndex == 2)
	return acl_check(acl_file, name);
#endif
    return 0;
}

int
sec_add_superuser(char *user)
{
#if defined(KERBEROS) && defined(HAVE_KRB4)
    return acl_add(acl_file, user);
#else
    return -1;
#endif
}

int
sec_del_superuser(char *user)
{
#if defined(KERBEROS) && defined(HAVE_KRB4)
    return acl_delete(acl_file, user);
#else
    return -1;
#endif
}

/*
 * Get the parsed name of a connection.
 * name, instance and realm should be properly allocated
 * Returns zero on success
 */

int
sec_getname(struct rx_connection *conn, char **name)
{
#if defined(KERBEROS) && defined(HAVE_KRB4)
    if (conn->securityIndex == 2) {
	serv_con_data *cdat;

	cdat = conn->securityData;

	return rxkad_GetServerInfoNew(conn, NULL, NULL, name, NULL);
    }
#endif
    return -1;
}

