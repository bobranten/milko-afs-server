/*
 * Copyright (c) 1995 - 1998, 2002 Kungliga Tekniska Högskolan
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

/* $Id: rxgk_locl.h,v 1.10 2007/05/16 20:19:39 lha Exp $ */

#ifndef __RXGK_LOCL_H
#define __RXGK_LOCL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_AFSCONFIG_H
#include <afsconfig.h>
#endif
#ifdef HAVE_AFS_PARAM_H
#include <afs/param.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <sys/types.h>
#include <netinet/in.h>

#include <gssapi/gssapi.h>
#include <gssapi/gssapi_krb5.h>
#include <krb5.h>

#include <errno.h>
#include "rxgk_err.h"

#include "rxgk_proto.h"



#ifdef NDEBUG
#ifndef assert
#define assert(e) ((void)0)
#endif
#else
#ifndef assert
#define assert(e) ((e) ? (void)0 : (void)osi_Panic("assert(%s) failed: file %s, line %d\n", #e, __FILE__, __LINE__, #e))
#endif
#endif

#undef RCSID
#include <rx/rx.h>
#include <rx/rx_null.h>
#undef RCSID
#define RCSID(msg) \
static /**/const char *const rcsid[] = { (char *)rcsid, "\100(#)" msg }

extern int rx_epoch, rx_nextCid;

#include "rxgk.h"

#define rxgk_disipline 3

#define rxgk_unallocated 1
#define rxgk_authenticated 2
#define rxgk_expired 4
#define rxgk_checksummed 8

#define RXGK_Ticket_Crypt_len len
#define RXGK_Ticket_Crypt_val val
#define RXGK_Token_len len
#define RXGK_Token_val val
#define rr_ctext_len len
#define rr_ctext_val val


extern krb5_context _rxgkk5ctx;

#if defined(AFS_HCRYPTO)
#include <rxgk_hcrypto.h>
#elif defined(AFS_RCRYPTO)
#include <rxgk_rcrypto.h>
#else
#error "select crypto"
#endif

typedef struct end_stuff {
    /* need 64 bit counters */
    uint32_t bytesReceived, packetsReceived, bytesSent, packetsSent;
} end_stuff;

extern int rxgk_key_contrib_size;

void
rxgk_crypto_start(void);

int
rxgk_prepare_packet(struct rx_packet *pkt, struct rx_connection *con,
		    int level, key_stuff *k, end_stuff *e,
		    int keyusage_enc, int keyusage_mic);

int
rxgk_check_packet(struct rx_packet *pkt, struct rx_connection *con,
		  int level, key_stuff *k, end_stuff *e,
		  int keyusage_enc, int keyusage_mic);

int
rxgk_crypto_init(struct rxgk_keyblock *tk, key_stuff *k);

/* rxgk */

int
rxgk_set_conn(struct rx_connection *, int, int);

int
rxgk_decode_auth_token(void *data, size_t len, struct rxgk_ticket *ticket);

int
rxgk_server_init(void);

int
rxgk_random_to_key(int, void *, int, krb5_keyblock *);

int
rxgk_make_ticket(struct rxgk_server_params *params,
		 gss_ctx_id_t ctx,
		 const void *snonce, size_t slength,
		 const void *cnonce, size_t clength,
		 RXGK_Ticket_Crypt *token,
		 int32_t enctype);

int
rxgk_get_gss_cred(uint32_t, short, gss_name_t, gss_name_t, int32_t,
		  int32_t *, time_t *, rxgk_level *, 
		  RXGK_Ticket_Crypt *, struct rxgk_keyblock *);

int
rxgk_encrypt_ticket(struct rxgk_ticket *ticket, RXGK_Ticket_Crypt *opaque);

int
rxgk_decrypt_ticket(RXGK_Ticket_Crypt *opaque, struct rxgk_ticket *ticket);

void
_rxgk_gssapi_err(OM_uint32, OM_uint32, gss_OID);

int
rxgk_derive_k0(gss_ctx_id_t ctx,
	       const void *snonce, size_t slength,
	       const void *cnonce, size_t clength,
	       int32_t enctype, struct rxgk_keyblock *key);

void
print_chararray(char *val, unsigned len);

int
rxgk_encrypt_buffer(RXGK_Token *in, RXGK_Token *out,
		    struct rxgk_keyblock *key, int keyusage);

int
rxgk_decrypt_buffer(RXGK_Token *in, RXGK_Token *out,
		    struct rxgk_keyblock *key, int keyusage);

int
rxgk_get_server_ticket_key(struct rxgk_keyblock *key);

int
rxgk_derive_transport_key(struct rxgk_keyblock *k0,
			  struct rxgk_keyblock *tk,
			  uint32_t epoch, uint32_t cid, int64_t start_time);

#endif /* __RXGK_LOCL_H */
