/*
 * Copyright (c) 1995-1997, 2003-2006 Kungliga Tekniska Högskolan
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

RCSID("$Id: rxk_serv.c,v 1.28 2006/11/13 17:10:58 tol Exp $");

#ifdef HAVE_KRB5
#include <krb5.h>
#endif

#undef HAVE_KRB4

#ifdef HAVE_KRB4
#include <krb.h>
#else
#include <krb4.h>
#endif

#ifndef KRB5_STORAGE_PRINCIPAL_WRONG_NUM_COMPONENTS
#undef HAVE_KRB5 /* no heimdal, no kerberos support */
#endif

#ifdef HAVE_OPENSSL
#include <openssl/rand.h>
#endif

/* Security object specific server data */
typedef struct rxkad_serv_class {
    struct rx_securityClass klass;
    rxkad_level min_level;
    void *appl_data;
    int (*get_key)(void *appl_data, int kvno, struct ktc_encryptionKey *key);
    int (*user_ok)(char *name, int kvno);
} rxkad_serv_class;

static
int
server_NewConnection(struct rx_securityClass *obj, struct rx_connection *con)
{
    assert(con->securityData == 0);
    obj->refCount++;
    con->securityData = (char *) osi_Alloc(sizeof(serv_con_data));
    memset(con->securityData, 0x0, sizeof(serv_con_data));
    return 0;
}

static
int
server_Close(struct rx_securityClass *obj)
{
    obj->refCount--;
    if (obj->refCount <= 0)
	osi_Free(obj, sizeof(rxkad_serv_class));
    return 0;
}

static
int
server_DestroyConnection(struct rx_securityClass *obj,
			 struct rx_connection *con)
{
    serv_con_data *cdat = (serv_con_data *)con->securityData;

    if (cdat)
    {
	if (cdat->user)
	    free(cdat->user);
	free(cdat);
    }
    return server_Close(obj);
}

/*
 * Check whether a connection authenticated properly.
 * Zero is good (authentication succeeded).
 */
static
int
server_CheckAuthentication(struct rx_securityClass *obj,
			   struct rx_connection *con)
{
    serv_con_data *cdat = (serv_con_data *) con->securityData;

    if (cdat)
	return !cdat->authenticated;
    else
	return RXKADNOAUTH;
}

/*
 * Select a nonce for later use.
 */
static
int
server_CreateChallenge(struct rx_securityClass *obj_,
		       struct rx_connection *con)
{
    rxkad_serv_class *obj = (rxkad_serv_class *) obj_;
    serv_con_data *cdat = (serv_con_data *) con->securityData;

    /* Any good random numbers will do, no real need to use
     * cryptographic techniques here */

#ifdef HAVE_OPENSSL
    uint32_t rnd[2];
    RAND_pseudo_bytes((void *)rnd, sizeof(rnd));
    cdat->nonce = rnd[0] ^ rnd[1];
#else
    union {
	uint32_t rnd[2];
	DES_cblock k;
    } u;
    DES_random_key(&u.k);
    cdat->nonce = u.rnd[0] ^ u.rnd[1];
#endif
   
    cdat->authenticated = 0;
    cdat->cur_level = obj->min_level;
    return 0;
}

/*
 * Wrap the nonce in a challenge packet.
 */
static
int
server_GetChallenge(const struct rx_securityClass *obj,
		    const struct rx_connection *con,
		    struct rx_packet *pkt)
{
    serv_con_data *cdat = (serv_con_data *) con->securityData;
    rxkad_challenge c;

    /* Make challenge */
    c.version = htonl(RXKAD_VERSION);
    c.nonce = htonl(cdat->nonce);
    c.min_level = htonl((int32_t)cdat->cur_level);
    c.unused = 0; /* Use this to hint client we understand krb5 tickets??? */

    /* Stuff into packet */
    if (rx_SlowWritePacket(pkt, 0, sizeof(c), &c) != sizeof(c))
	return RXKADPACKETSHORT;
    rx_SetDataSize(pkt, sizeof(c));
    return 0;
}

static
int
decode_krb5_ticket(rxkad_serv_class *obj,
		   int serv_kvno,
		   void *ticket,
		   int32_t ticket_len,
		   /* OUT parms */
		   struct ktc_encryptionKey *session_key,
		   uint32_t *expires,
		   char **user)
{
#ifndef HAVE_KRB5
    return RXKADBADTICKET;
#else
    struct ktc_encryptionKey serv_key; /* Service's secret key */
    krb5_keyblock key;		/* Uses serv_key above */
    int code;
    size_t siz;
    krb5_principal principal;
    krb5_crypto crypto;

    Ticket t5;			/* Must free */
    EncTicketPart decr_part;	/* Must free */
    krb5_context context;		/* Must free */
    krb5_data plain;		/* Must free */

    memset(&t5, 0x0, sizeof(t5));
    memset(&decr_part, 0x0, sizeof(decr_part));
    krb5_init_context(&context);
    krb5_data_zero(&plain);

    assert(serv_kvno == RXKAD_TKT_TYPE_KERBEROS_V5 ||
	   serv_kvno == RXKAD_TKT_TYPE_KERBEROS_V5_ENCPART_ONLY);

    if (serv_kvno == RXKAD_TKT_TYPE_KERBEROS_V5_ENCPART_ONLY) {
	code = decode_EncryptedData(ticket, ticket_len, &t5.enc_part, &siz);
	if (code != 0)
	    goto bad_ticket;

	serv_kvno = 0;
    } else {
	code = decode_Ticket(ticket, ticket_len, &t5, &siz);
	if (code != 0)
	    goto bad_ticket;

	serv_kvno = t5.tkt_vno;
    }

    /* Check that the key type really fit into 8 bytes */
    switch (t5.enc_part.etype) {
    case ETYPE_DES_CBC_CRC:
    case ETYPE_DES_CBC_MD4:
    case ETYPE_DES_CBC_MD5:
	key.keytype = t5.enc_part.etype;
	key.keyvalue.length = 8;
	key.keyvalue.data = serv_key.data;
	break;
    default:
	goto unknown_key;
    }
  
    /* Get the service key. We have to assume that the key type is of
     * size 8 bytes or else we can't store service keys for both krb4
     * and krb5 in the same way in /usr/afs/etc/KeyFile.
     */
    code = (*obj->get_key)(obj->appl_data, serv_kvno, &serv_key);
    if (code)
	goto unknown_key;

    code = krb5_crypto_init(context, &key, 0, &crypto);
    if (code)
	goto unknown_key;

    /* Decrypt ticket */
    code = krb5_decrypt(context,
			crypto,
			0,
			t5.enc_part.cipher.data,
			t5.enc_part.cipher.length,
			&plain);
    krb5_crypto_destroy(context, crypto);
    if (code != 0)
	goto bad_ticket;
  
    /* Decode ticket */
    code = decode_EncTicketPart(plain.data, plain.length, &decr_part, &siz);
    if (code != 0)
	goto bad_ticket;

    /* Check that the key type really fit into 8 bytes */
    switch (decr_part.key.keytype) {
    case ETYPE_DES_CBC_CRC:
    case ETYPE_DES_CBC_MD4:
    case ETYPE_DES_CBC_MD5:
	break;
    default:
	goto unknown_key;
    }

    {
	principal = calloc(1, sizeof(*principal));
	if (principal == NULL) {
	    code = ENOMEM;
	    goto cleanup;
	}
	code = copy_PrincipalName(&decr_part.cname, &principal->name);
	if (code)
	    goto cleanup;
	principal->realm = strdup(decr_part.crealm);
	if (principal->realm == NULL) {
	    code = ENOMEM;
	    goto cleanup;
	}
    }

    /* Extract realm and principal */  
    code = krb5_unparse_name(context, principal, user);
    krb5_free_principal(context, principal);
    if (code)
	goto cleanup;
  
    /* Extract session key */
    memcpy(session_key->data, decr_part.key.keyvalue.data, 8);

    /* Check lifetimes and host addresses, flags etc */
    {
	time_t now = time(0);	/* Use fast time package instead??? */
	time_t start = decr_part.authtime;
	time_t end = decr_part.endtime;

	if (decr_part.starttime)
	    start = *decr_part.starttime;

	if (start >= end
	    || start > now + KTC_TIME_UNCERTAINTY
	    || decr_part.flags.invalid)
	    goto no_auth;

	if (now > end + KTC_TIME_UNCERTAINTY)
	    goto tkt_expired;

	*expires = end;
    }

    code = 0;

#if 0
    /* Check host addresses */
#endif

 cleanup:
    if (code) {
	free(*user);
	*user = NULL;
    }
    free_Ticket(&t5);
    free_EncTicketPart(&decr_part);
    krb5_free_context(context);
    krb5_data_free(&plain);
    return code;
  
 unknown_key:
    code = RXKADUNKNOWNKEY;
    goto cleanup;
 no_auth:
    code = RXKADNOAUTH;
    goto cleanup;
 tkt_expired:
    code = RXKADEXPIRED;
    goto cleanup;
 bad_ticket:
    code = RXKADBADTICKET;
    goto cleanup;
#endif /* HAVE_KRB5 */
}

static
int
decode_krb4_ticket(rxkad_serv_class *obj,
		   int serv_kvno,
		   void *ticket,
		   int32_t ticket_len,
		   /* OUT parms */
		   struct ktc_encryptionKey *session_key,
		   uint32_t *expires,
		   char **user)
{
#if !defined(HAVE_KRB4) && !defined(HAVE_KRB5)
    return RXKADBADTICKET;
#else
    u_char kflags;
    int klife;
    uint32_t start;
    uint32_t paddress;
    char sname[SNAME_SZ], sinstance[INST_SZ];
    char name[ANAME_SZ], instance[INST_SZ], realm[REALM_SZ];
    KTEXT_ST tkt;
    struct ktc_encryptionKey serv_key; /* Service's secret key */
    DES_cblock key;
    DES_key_schedule serv_sched;	/* Service's schedule */

    /* First get service key */
    int code = (*obj->get_key)(obj->appl_data, serv_kvno, &serv_key);
    if (code)
	return RXKADUNKNOWNKEY;

    memcpy(&key, serv_key.data, sizeof(key));
    DES_key_sched(&key, &serv_sched);
    tkt.length = ticket_len;
    memcpy(tkt.dat, ticket, ticket_len);
    code = decomp_ticket(&tkt, &kflags,
			 name, instance, realm, &paddress,
			 (void*)session_key->data, &klife, &start,
			 sname, sinstance,
			 (DES_cblock *)&serv_key, &serv_sched);
    if (code != KSUCCESS)
	return RXKADBADTICKET;

#if 0
    if (paddress != ntohl(con->peer->host))
	return RXKADBADTICKET;
#endif

    {
#ifndef HAVE_KRB4
	time_t end = krb4_life_to_time(start, klife);
#else
	time_t end = krb_life_to_time(start, klife);
#endif
	time_t now = time(0);
	if (start > CLOCK_SKEW) /* Transarc sends 0 as start if localauth */
	    start -= CLOCK_SKEW;
	if (now < start)
	    return RXKADNOAUTH;
	else if (now > end)
	    return RXKADEXPIRED;
	*expires = end;
    }
    asprintf(user, "%s%s%s@%s", 
	     name, 
	     instance[0] ? "." : "",
	     instance[0] ? instance : "",
	     realm);
    if (*user == NULL)
	return ENOMEM;
    return 0;			/* Success */
#endif /* HAVE_KRB4 */
}

/*
 * Process a response to a challange.
 */
static
int
server_CheckResponse(struct rx_securityClass *obj_,
		     struct rx_connection *con,
		     struct rx_packet *pkt)
{
    rxkad_serv_class *obj = (rxkad_serv_class *) obj_;
    serv_con_data *cdat = (serv_con_data *) con->securityData;

    int serv_kvno;		/* Service's kvno we used */
    int32_t ticket_len;
    char ticket[MAXKRB5TICKETLEN];
    int code;
    rxkad_response r;
    char *user = NULL;
    uint32_t cksum;

    if (rx_SlowReadPacket(pkt, 0, sizeof(r), &r) != sizeof(r))
	return RXKADPACKETSHORT;
  
    serv_kvno = ntohl(r.kvno);
    ticket_len = ntohl(r.ticket_len);

    if (ticket_len > MAXKRB5TICKETLEN)
	return RXKADTICKETLEN;

    if (rx_SlowReadPacket(pkt, sizeof(r), ticket_len, ticket) != ticket_len)
	return RXKADPACKETSHORT;

    /* Disassemble kerberos ticket */
    if (serv_kvno == RXKAD_TKT_TYPE_KERBEROS_V5 ||
	serv_kvno == RXKAD_TKT_TYPE_KERBEROS_V5_ENCPART_ONLY)
	code = decode_krb5_ticket(obj, serv_kvno, ticket, ticket_len,
				  &cdat->k.key, &cdat->expires, &user);
    else
	code = decode_krb4_ticket(obj, serv_kvno, ticket, ticket_len,
				  &cdat->k.key, &cdat->expires, &user);
    if (code != 0)
	return code;

    fc_keysched(cdat->k.key.data, cdat->k.keysched);

    /* Unseal r.encrypted */
    fc_cbc_enc2(&r.encrypted, &r.encrypted, sizeof(r.encrypted),
		cdat->k.keysched, (uint32_t*)cdat->k.key.data, FC_DECRYPT);

    /* Verify response integrity */
    cksum = r.encrypted.cksum;
    r.encrypted.cksum = 0;
    if (r.encrypted.epoch != ntohl(con->epoch)
	|| r.encrypted.cid != ntohl(con->cid & RX_CIDMASK)
	|| r.encrypted.security_index != ntohl(con->securityIndex)
	|| cksum != rxkad_cksum_response(&r)) {
	free(user);
	return RXKADSEALEDINCON;
    }
    {
	int i;
	for (i = 0; i < RX_MAXCALLS; i++)
	{
	    r.encrypted.call_numbers[i] = ntohl(r.encrypted.call_numbers[i]);
	    if (r.encrypted.call_numbers[i] < 0) {
		free(user);
		return RXKADSEALEDINCON;
	    }
	}
    }

    if (ntohl(r.encrypted.inc_nonce) != cdat->nonce+1) {
	free(user);
	return RXKADOUTOFSEQUENCE;
    }

    {
	int level = ntohl(r.encrypted.level);
	if ((level < cdat->cur_level) || (level > rxkad_crypt)) {
	    free(user);
	    return RXKADLEVELFAIL;
	}
	cdat->cur_level = level;
	/* We don't use trailers but the transarc implementation breaks if
	 * we don't set the trailer size, packets get to large */
	if (level == rxkad_auth)
	{
	    rx_SetSecurityHeaderSize(con, 4);
	    rx_SetSecurityMaxTrailerSize(con, 4);
	}
	else if (level == rxkad_crypt)
	{
	    rx_SetSecurityHeaderSize(con, 8);
	    rx_SetSecurityMaxTrailerSize(con, 8);
	}
    }
  
    rxi_SetCallNumberVector(con, r.encrypted.call_numbers);

    rxkad_calc_header_iv(con, cdat->k.keysched,
			 (void *)&cdat->k.key, cdat->e.header_iv);
    cdat->authenticated = 1;

    cdat->user = user;
    if (obj->user_ok)
    {
	code = obj->user_ok(user, serv_kvno);
	if (code)
	    return RXKADNOAUTH;
    }

    return 0;
}

/*
 * Checksum and/or encrypt packet
 */
static
int
server_PreparePacket(struct rx_securityClass *obj_,
		     struct rx_call *call,
		     struct rx_packet *pkt)
{
    struct rx_connection *con = rx_ConnectionOf(call);
    serv_con_data *cdat = (serv_con_data *) con->securityData;
    key_stuff *k = &cdat->k;
    end_stuff *e = &cdat->e;

    return rxkad_prepare_packet(pkt, con, cdat->cur_level, k, e);
}

/*
 * Verify checksum and/or decrypt packet.
 */
static
int
server_CheckPacket(struct rx_securityClass *obj_,
		   struct rx_call *call,
		   struct rx_packet *pkt)
{
    struct rx_connection *con = rx_ConnectionOf(call);
    serv_con_data *cdat = (serv_con_data *) con->securityData;
    key_stuff *k = &cdat->k;
    end_stuff *e = &cdat->e;

    if (time(0) > cdat->expires)	/* Use fast time package instead??? */
	return RXKADEXPIRED;

    return rxkad_check_packet(pkt, con, cdat->cur_level, k, e);
}

static
int
server_GetStats(const struct rx_securityClass *obj_,
		const struct rx_connection *con,
		struct rx_securityObjectStats *st)
{
    rxkad_serv_class *obj = (rxkad_serv_class *) obj_;
    serv_con_data *cdat = (serv_con_data *) con->securityData;

    st->type = rxkad_disipline;
    st->level = obj->min_level;
    st->flags = rxkad_checksummed;
    if (cdat == 0)
	st->flags |= rxkad_unallocated;
    {
	st->bytesReceived = cdat->e.bytesReceived;
	st->packetsReceived = cdat->e.packetsReceived;
	st->bytesSent = cdat->e.bytesSent;
	st->packetsSent = cdat->e.packetsSent;
	st->expires = cdat->expires;
	st->level = cdat->cur_level;
	if (cdat->authenticated)
	    st->flags |= rxkad_authenticated;
    }
    return 0;
}

static struct rx_securityOps server_ops = {
    server_Close,
    server_NewConnection,
    server_PreparePacket,
    0,
    server_CheckAuthentication,
    server_CreateChallenge,
    server_GetChallenge,
    0,
    server_CheckResponse,
    server_CheckPacket,
    server_DestroyConnection,
    server_GetStats,
};

struct rx_securityClass *
rxkad_NewServerSecurityObjectNew(/*rxkad_level*/ int min_level,
				 void *appl_data,
				 int (*get_key)(void *appl_data,
						int kvno,
						struct ktc_encryptionKey *key),
				 int (*user_ok)(char *name,
						int kvno))
{
    rxkad_serv_class *obj;

    if (!get_key)
	return 0;

    obj = (rxkad_serv_class *) osi_Alloc(sizeof(rxkad_serv_class));
    obj->klass.refCount = 1;
    obj->klass.ops = &server_ops;
    obj->klass.privateData = (char *) obj;

    obj->min_level = min_level;
    obj->appl_data = appl_data;
    obj->get_key = get_key;
    obj->user_ok = user_ok;

    return &obj->klass;
}
