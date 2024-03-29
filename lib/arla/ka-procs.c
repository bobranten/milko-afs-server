/*
 * Copyright (c) 2000 - 2001, 2004 Kungliga Tekniska H�gskolan
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

/*
 * This file implements wrapper functions around the interfaces
 * (KAA,KAT,KAM) that is available from ka-server.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

RCSID("$Id: ka-procs.c,v 1.27 2007/01/16 11:46:09 tol Exp $");

#include <errno.h>
#include <sys/types.h>

#include <ka-procs.h>

#ifdef KASERVER_SUPPORT

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include <openssl/des.h>
#ifdef HAVE_OPENSSL_UI_H
#include <openssl/ui.h>
#endif
#include <openssl/opensslv.h>

#include <ko.h>
#include <ports.h>
#include <log.h>

#include <arlalib.h>

#include <roken.h>
#include <err.h>

#include <service.h>

#include <rx/rx.h>
#include <ka.h>
#include <ka.cs.h>

typedef struct {
    char dat[MAXKTCTICKETLEN];
    size_t length;
} ka_ticket_t;

struct ka_Answer {
    uint32_t challange;
    unsigned char sessionkey[8];
    uint32_t start_time;
    uint32_t end_time;
    int kvno;
    char user[MAXKANAMELEN];
    char instance[MAXKANAMELEN];
    char realm[MAXKANAMELEN];
    char server_user[MAXKANAMELEN];
    char server_instance[MAXKANAMELEN];
    ka_ticket_t ticket;
    char label[KA_LABELSIZE];
};

typedef struct ka_Answer ka_auth_data_t;
typedef struct ka_Answer ka_ticket_data_t;

/*
 * The ka preauth data-blob. Used in KAA_Authenticate. Encrypted with
 * DES_pcbc_encrypt() and the user's key.
 */

struct ka_preauth {
    int32_t time;
    char label[KA_LABELSIZE];
};

/*
 * Used to communicate over start and endtime of a ticket in
 * KAT_GetTicket(). Encrypted with DES_ecb_encrypt (for some reson)
 * and the session key.
 */

struct ka_times {
    int32_t start_time;
    int32_t end_time;
};


/*
 *
 */

static void
decode_u_int32 (uint32_t *number, unsigned char **buf, size_t *sz)
{
    memcpy (number, *buf, 4);
    *number = ntohl(*number);
    *sz -= 4; *buf += 4;
}

/*
 *
 */

static void
decode_stringz (char *str, size_t str_sz, unsigned char **buf, size_t *sz)
{
    char *s = (char *)*buf;
    size_t l = strlen (s) + 1;
    if (l > str_sz) abort();
    strlcpy (str, s, l);
    *sz -= l; *buf += l;
}

/*
 * Decode the getticket
 *
 * confounder			int32_t
 * challange			int32_t
 * sessionkey			byte * 8
 * start time			int32_t
 * end time			int32_t
 * kvno				int32_t
 * ticket length		int32_t
 * client name			stringz
 * client instance		stringz
 * realm ?			stringz
 * server name			stringz
 * server instance		stringz
 * ticket			byte * <ticket length>
 * label (tgsT in tgt case)	byte * 4
 * padding to make sure it's on a 8 byte bondery
 *
 * blam heimdal authors if it's wrong
 */

static int
decode_answer (char *label, unsigned char *buf, size_t sz,
	       struct ka_Answer *answer)
{
    uint32_t confounder, kvno_int32, ticket_sz;
    
    assert (sz % 8 == 0);
    
    if (sz < 32 + 1+1+1 + 1+1)
	goto fail;
    
    decode_u_int32 (&confounder, &buf, &sz);
    decode_u_int32 (&answer->challange, &buf, &sz);
    memcpy (answer->sessionkey, buf, 8); 
    buf += 8; sz -= 8;
    decode_u_int32 (&answer->start_time, &buf, &sz);
    decode_u_int32 (&answer->end_time, &buf, &sz);
    decode_u_int32 (&kvno_int32, &buf, &sz);
    answer->kvno = (int)kvno_int32;
    decode_u_int32 (&ticket_sz, &buf, &sz);
    if (ticket_sz > MAXKTCTICKETLEN)
	goto fail;
    
#define destrz(f) decode_stringz(answer->f,sizeof(answer->f), &buf, &sz)
    destrz(user);
    destrz(instance);
    destrz(realm);
    destrz(server_user);
    destrz(server_instance);
#undef destrz
    
    if (sz < ticket_sz)
	goto fail;

    answer->ticket.length = ticket_sz;
    memcpy (&answer->ticket.dat, buf, ticket_sz);
    sz -= ticket_sz;
    buf += ticket_sz;
    
    if (sz < 4)
	goto fail;
    memcpy (answer->label, buf, 4);
    sz -= 4;
    buf += 4;

    if (memcmp (answer->label, label, 4) != 0)
	goto fail;
    
    assert (sz >= 0);

    return 0;
 fail:
    memset(answer->sessionkey, 0, sizeof(answer->sessionkey));
    return 1;
}

#define lowcase(c) (('A' <= (c) && (c) <= 'Z') ? ((c) - 'A' + 'a') : (c))

/*
 * The string to key function used by Transarc AFS. Stolen from kth-krb.
 */
static void
afs_string_to_key(const char *pass, const char *cell, DES_cblock *key)
{
  if (strlen(pass) <= 8)	/* Short passwords. */
    {
      char buf[8 + 1], *s;
      int i;

      /*
       * XOR cell and password and pad (or fill) with 'X' to length 8,
       * then use crypt(3) to create DES key.
       */
      for (i = 0; i < 8; i++)
	{
	  buf[i] = *pass ^ lowcase(*cell);
	  if (buf[i] == 0)
	    buf[i] = 'X';
	  if (*pass != 0)
	    pass++;
	  if (*cell != 0)
	    cell++;
	}
      buf[8] = 0;

      s = crypt(buf, "p1");	/* Result from crypt is 7bit chars. */
      s = s + 2;		/* Skip 2 chars of salt. */
      for (i = 0; i < 8; i++)
	((char *) key)[i] = s[i] << 1; /* High bit is always zero */
      DES_fixup_key_parity(key); /*       Low  bit is parity */
    }
  else				/* Long passwords */
    {
      int plen, clen;
      char *buf, *t;
      DES_key_schedule sched;
      DES_cblock ivec;

      /*
       * Concatenate password with cell name,
       * then checksum twice to create DES key.
       */
      plen = strlen(pass);
      clen = strlen(cell);
      buf = malloc(plen + clen + 1);
      memcpy(buf, pass, plen);
      for (t = buf + plen; *cell != 0; t++, cell++)
	*t = lowcase(*cell);

      memcpy(&ivec, "kerberos", 8);
      memcpy(key, "kdsbdsns", 8);
      DES_key_sched(key, &sched);
      /* Beware, ivec is passed twice */
      DES_cbc_cksum(buf, &ivec, plen + clen, &sched, &ivec);

      memcpy(key, &ivec, 8);
      DES_fixup_key_parity(key);
      DES_key_sched(key, &sched);
      /* Beware, ivec is passed twice */
      DES_cbc_cksum(buf, key, plen + clen, &sched, &ivec);
      free(buf);
      DES_fixup_key_parity(key);
    }
}

/*
 * Get the the password des-`key' for `realm' (and its make sure that
 * the cell is lowercase) using `password'. If `password' is NULL, the
 * user is queried.
 */

static int
get_password (const char *realm, const char *password, DES_cblock *key)
{
    int ret;
    char buf[1024];
    char cell[1024];

    if (password == NULL) {
	ret = UI_UTIL_read_pw_string(buf, sizeof(buf), "Password: ", 0);
	if (ret)
	    return ret;
	password = buf;
    }
    strlcpy (cell, realm, sizeof(cell));
    strlwr (cell);

    afs_string_to_key (password, realm, key);
    memset (buf, 0, sizeof(buf));

    return 0;
}

/*
 * Make sure `answer' have a good realm, or set to it `realm'
 */

static void
fixup_realm (struct ka_Answer *answer, const char *realm)
{
    if (strcmp (answer->realm, "") == 0) {
	strlcpy(answer->realm, realm, 
		sizeof(answer->realm));
	strupr(answer->realm);
    }
}

static void
build_cleartoken(ka_ticket_data_t *tdata, struct ClearToken *ct)
{
    ct->AuthHandle = tdata->kvno;
    memcpy(ct->HandShakeKey,
	   tdata->sessionkey, sizeof(tdata->sessionkey));
    ct->ViceId = 0;
    ct->BeginTimestamp = tdata->start_time;
    ct->EndTimestamp = tdata->end_time;

    /* traditional: diff must be even */
    if ((ct->EndTimestamp - ct->BeginTimestamp) & 1)
	ct->EndTimestamp--;
}

#ifndef DES_ENCRYPT
#if defined(DESKRB_ENCRYPT)
#define DES_ENCRYPT DESKRB_ENCRYPT
#define DES_DECRYPT DESKRB_DECRYPT
#elif defined(ENCRYPT)
#define DES_ENCRYPT ENCRYPT
#define DES_DECRYPT DECRYPT
#else
#error DES_ENCRYPT not defined in des.h
#endif
#endif

/*
 * Authenticate `user'.`instance'@`cell' with `key' to get a tgt for
 * with a lifetime `lifetime' seconds. Return ticket in
 * `adata'.
 */

static int
ka_auth (const char *user, const char *instance, const char *cell,
	 DES_cblock *key, ka_auth_data_t *adata,
	 uint32_t lifetime)
{
    struct db_server_context conn_context;
    struct rx_connection *conn;
    struct ka_preauth f;
    struct timeval tv;
    char buf[8];
    char return_data[400];
    ka_CBS request;
    ka_BBS answer;
    int ret;
    struct ka_Answer *auth_answer = (struct ka_Answer *)adata;
    DES_key_schedule schedule;
    uint32_t req_challange;
    uint32_t start_time;
    uint32_t end_time;

    /*
     * Init the time-stamps
     */

    gettimeofday (&tv, NULL);

    start_time = tv.tv_sec;
    end_time = start_time + lifetime;

    /*
     * Reset stuff that is sent over the network
     */

    memset(buf, 0, sizeof(buf));
    memset(return_data, 0, sizeof(return_data));

    /*
     * Cook pre-auth
     */

    f.time = htonl(tv.tv_sec);
    strncpy (f.label, "gTGS", 4);
    req_challange = tv.tv_sec;

    DES_set_key (key, &schedule);
    DES_pcbc_encrypt ((unsigned char *)&f, 
		      buf,
		      sizeof(f), 
		      &schedule,
		      key, 
		      DES_ENCRYPT);

    request.Seq.val = buf;
    request.Seq.len = sizeof(buf);

    /*
     * Setup query stuff
     */

    answer.MaxSeqLen = sizeof(buf);
    answer.Seq.val   = return_data;
    answer.Seq.len   = sizeof(return_data);
    answer.MaxSeqLen = sizeof(return_data);

    /*
     * Send the request
     */
    
    ret = ENETDOWN;
    for (conn = arlalib_first_db(&conn_context,
				 cell, NULL, afskaport, 
				 KA_AUTHENTICATION_SERVICE, 
				 arlalib_getauthflag (1, 0, 0, 0));
	 conn != NULL && arlalib_try_next_db(ret);
	 conn = arlalib_next_db(&conn_context)) {

	ret = KAA_Authenticate (conn, user, instance, start_time, end_time,
				&request, &answer);
    }
    free_db_server_context(&conn_context);
    if (ret) 
	return ret;

    /* 
     * Decrypt it
     */

    DES_set_key (key, &schedule);
    DES_pcbc_encrypt ((unsigned char *)answer.Seq.val,
		      (unsigned char *)answer.Seq.val,
		      answer.Seq.len,
		      &schedule,
		      key,
		      DES_DECRYPT);
    memset (key, 0, sizeof(*key));
    
    ret = decode_answer ("tgsT", answer.Seq.val, answer.Seq.len,
			 auth_answer);
    memset (return_data, 0, sizeof (return_data));
    memset (answer.Seq.val, 0, answer.Seq.len);
    free (answer.Seq.val);
    if (ret)
	return ret;


    /*
     * Consistency checks
     */

    if (strcmp(auth_answer->user, user) != 0 
	|| strcmp(auth_answer->instance, instance) != 0) {
	printf ("server returned different user the asked for\n");
	return 1;
    }
    
    if (auth_answer->challange != req_challange + 1) {
	printf ("server didn't respond with right challange\n");
	return 1;
    }

    fixup_realm (auth_answer, cell);
     
    return ret;
}

/*
 * Get a ticket for `suser'.`sinstance'@`srealm' using `adata' to
 * authenticate. The returning ticket is retrurned in `tdata'.
 */

static int
ka_getticket (const char *suser, const char *sinstance, const char *srealm,
	      ka_auth_data_t *adata, ka_ticket_data_t *tdata)
{
    struct db_server_context conn_context;
    struct rx_connection *conn;
    int ret;
    struct ka_times ktimes;
    char times_buf[8];
    char return_data[400];
    ka_CBS times;
    ka_CBS ka_ticket;
    ka_BBS answer;
    DES_key_schedule schedule;
    struct timeval tv;
    uint32_t start_time;
    uint32_t end_time;

    /*
     * Cook time limit of ticket, use enddate of tgt
     */

    gettimeofday (&tv, NULL);

    start_time = tv.tv_sec;
    end_time = adata->end_time;

    ktimes.start_time = htonl(start_time);
    ktimes.end_time = htonl(end_time);

    DES_set_key (&adata->sessionkey, &schedule);
    DES_ecb_encrypt ((DES_cblock *)&ktimes,
		     (DES_cblock *)times_buf,
		     &schedule,
		     DES_ENCRYPT);		      

    /*
     * Set up request
     */

    times.Seq.len = sizeof(times_buf);
    times.Seq.val = times_buf;

    ka_ticket.Seq.len = adata->ticket.length;
    ka_ticket.Seq.val = adata->ticket.dat;
    
    answer.MaxSeqLen = sizeof(return_data);
    answer.Seq.len = sizeof(return_data);
    answer.Seq.val = return_data;

    ret = ENETDOWN;
    for (conn = arlalib_first_db(&conn_context,
				 adata->realm, NULL, afskaport, 
				 KA_TICKET_GRANTING_SERVICE, 
				 arlalib_getauthflag (1, 0, 0, 0));
	 conn != NULL && arlalib_try_next_db(ret);
	 conn = arlalib_next_db(&conn_context)) {

	ret = KAT_GetTicket(conn, adata->kvno, "", &ka_ticket,
			    suser, sinstance, &times, &answer);
    }
    free_db_server_context(&conn_context);
    if (ret) 
	return ret;

    /* decrypt it */
    DES_set_key (&adata->sessionkey, &schedule);
    DES_pcbc_encrypt ((unsigned char *)answer.Seq.val,
		      (unsigned char *)answer.Seq.val,
		      answer.Seq.len,
		      &schedule,
		      &adata->sessionkey,
		      DES_DECRYPT);

    ret = decode_answer ("gtkt", answer.Seq.val, answer.Seq.len, tdata); 
    if (ret)
	return ret;

    fixup_realm (tdata, adata->realm);
    return 0;
}

#endif /* KASERVER_SUPPORT */

/*
 * Authenticate `user'.`instance'@`cell' with `password' and get
 * `lifetime' second long tickets. If `password' is NULL, user is
 * queried for password. `flags' is used to specify behavior of the
 * function.
 */

int
ka_authenticate (const char *user, const char *instance, const char *cell,
		 const char *password, uint32_t lifetime)
{
#undef KASERVER_SUPPORT
#ifdef KASERVER_SUPPORT
    DES_cblock key;
    ka_auth_data_t adata;
    ka_ticket_data_t tdata;
    struct ClearToken ct;
    int ret;

    if (cell == NULL)
	cell = cell_getthiscell();
    if (instance == NULL)
	instance = "";

    ret = get_password (cell, password, &key);
    if (ret)
	goto out;

    ret = ka_auth (user, instance, cell, &key, &adata, lifetime);
    if (ret)
	goto out;

    ret = ka_getticket ("afs", "", cell, &adata, &tdata);
    if (ret)
	goto out;

    if (k_hasafs()) {
	build_cleartoken(&tdata, &ct);
	ret = kafs_settoken_rxkad(cell, &ct, &tdata.ticket.dat,
				  tdata.ticket.length);
	if (ret)
	    warnx ("failed inserting tokens for cell %s", cell);
    }

 out:
    memset (&key, 0, sizeof (key));
    memset (&adata, 0, sizeof(adata));
    memset (&tdata, 0, sizeof(tdata));

    return ret;
#else
    return EINVAL;
#endif
}

