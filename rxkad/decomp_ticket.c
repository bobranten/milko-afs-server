/*
 * Copyright (c) 1995-2000 Kungliga Tekniska Högskolan
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

#include "rxkad_locl.h"

RCSID("$Id: decomp_ticket.c,v 1.2 2005/01/23 17:34:05 lha Exp $");

#ifndef HAVE_KRB4

#include "krb4.h"

/*
 * This routine takes a ticket and pointers to the variables that
 * should be filled in based on the information in the ticket.  It
 * fills in values for its arguments.
 *
 * The routine returns KFAILURE if any of the "pname", "pinstance",
 * or "prealm" fields is too big, otherwise it returns KSUCCESS.
 *
 * The corresponding routine to generate tickets is create_ticket.
 * When changes are made to this routine, the corresponding changes
 * should also be made to that file.
 *
 * See create_ticket.c for the format of the ticket packet.
 */

#ifdef HAVE_KRB5

#include <krb5.h>

typedef char* krb5_realm;

static int
get_lrealm(char *r, int n)
{
    krb5_error_code ret;
    krb5_context context;
    krb5_realm realm;

    if (n != 1)
	return KFAILURE;

    ret = krb5_init_context(&context);
    if (ret)
	return KFAILURE;

    ret = krb5_get_default_realm(context, &realm);
    if (ret) {
	krb5_free_context(context);
	return KFAILURE;
    }

    strlcpy(r, realm, REALM_SZ);
    free(realm);
    krb5_free_context(context);
    return KSUCCESS;
}

#else

static int
get_lrealm(char *r, int n)
{
    return KFAILURE;
}

#endif /* HAVE_KRB5 */

int
decomp_ticket(KTEXT tkt,	/* The ticket to be decoded */
	      unsigned char *flags, /* Kerberos ticket flags */
	      char *pname,	/* Authentication name */
	      char *pinstance,	/* Principal's instance */
	      char *prealm,	/* Principal's authentication domain */
	      u_int32_t *paddress,/* Net address of entity requesting ticket */
	      unsigned char *session, /* Session key inserted in ticket */
	      int *life,	/* Lifetime of the ticket */
	      u_int32_t *time_sec, /* Issue time and date */
	      char *sname,	/* Service name */
	      char *sinstance,	/* Service instance */
	      DES_cblock *key,	/* Service's secret key (to decrypt the ticket) */
	      DES_key_schedule *schedule) /* The precomputed key schedule */

{
    unsigned char *p = tkt->dat;
    
    int little_endian;

    DES_pcbc_encrypt(tkt->dat, tkt->dat,
		     tkt->length, schedule, key, DES_DECRYPT);

    tkt->mbz = 0;

    *flags = *p++;

    little_endian = *flags & 1;

    if(strlen((char*)p) > ANAME_SZ)
	return KFAILURE;
    p += krb4_get_string(p, pname, ANAME_SZ);

    if(strlen((char*)p) > INST_SZ)
	return KFAILURE;
    p += krb4_get_string(p, pinstance, INST_SZ);

    if(strlen((char*)p) > REALM_SZ)
	return KFAILURE;
    p += krb4_get_string(p, prealm, REALM_SZ);

    if (*prealm == '\0')
	get_lrealm (prealm, 1);

    if(tkt->length - (p - tkt->dat) < 8 + 1 + 4)
	return KFAILURE;
    p += krb4_get_address(p, paddress);

    memcpy(session, p, 8);
    p += 8;

    *life = *p++;
    
    p += krb4_get_int(p, time_sec, 4, little_endian);

    if(strlen((char*)p) > SNAME_SZ)
	return KFAILURE;
    p += krb4_get_string(p, sname, SNAME_SZ);

    if(strlen((char*)p) > INST_SZ)
	return KFAILURE;
    p += krb4_get_string(p, sinstance, INST_SZ);

    return KSUCCESS;
}
#endif

