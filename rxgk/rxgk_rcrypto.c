/*
 * Copyright (c) 2007, Stockholms universitet
 * (Stockholm University, Stockholm Sweden)
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
 * 3. Neither the name of the university nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "rxgk_locl.h"
#include <errno.h>

RCSID("$Id: rxgk_rcrypto.c,v 1.1 2007/05/14 21:29:48 lha Exp $");

#ifdef AFS_RCRYPTO

void
rxgk_crypto_start(void)
{
}

int
rxgk_crypto_init(struct rxgk_keyblock *tk, key_stuff *k)
{
    return 0;
}

struct rxgk_pkg_hdr {
    uint32_t call_number;
    uint32_t channel_and_seq;
    uint32_t flag_user_status_svcid;
};

static void
getheader(struct rx_connection *conn, 
	  struct rx_packet *pkt,
	  struct rxgk_pkg_hdr *h)
{
    uint32_t t;

    /* Collect selected pkt fields */
    h->call_number = htonl(pkt->header.callNumber);
    t = ((pkt->header.cid & RX_CHANNELMASK) << (32 - RX_CIDSHIFT))
	| ((pkt->header.seq & 0x3fffffff));
    h->channel_and_seq = htonl(t);
    h->flag_user_status_svcid = htonl(conn->serviceId);
}

int
rxgk_prepare_packet(struct rx_packet *p, struct rx_connection *conn,
		    int level, key_stuff *k, end_stuff *e,
		    int keyusage_enc, int keyusage_mic)
{
    return 0;
}

/*
 *
 */
int
rxgk_check_packet(struct rx_packet *p, struct rx_connection *conn,
		  int level, key_stuff *k, end_stuff *e,
		  int keyusage_enc, int keyusage_mic)
{
    return 0;
}

int
rxgk_encrypt_buffer(RXGK_Token *in, RXGK_Token *out,
		    struct rxgk_keyblock *key, int keyusage)
{
    return 0;
}

int
rxgk_decrypt_buffer(RXGK_Token *in, RXGK_Token *out,
		    struct rxgk_keyblock *key, int keyusage)
{
    return 0;
}

int
rxgk_derive_transport_key(struct rxgk_keyblock *k0,
			  struct rxgk_keyblock *tk,
			  uint32_t epoch, uint32_t cid, int64_t start_time)
{
    int i;
    uint32_t x;
    /* XXX get real key */

    if (k0->enctype != RXGK_CRYPTO_AES256_CTS_HMAC_SHA1_96) {
	return EINVAL;
    }

    tk->length = 32;
    tk->enctype = RXGK_CRYPTO_AES256_CTS_HMAC_SHA1_96;
    
    tk->data = malloc(tk->length);
    if (tk->data == NULL)
	return ENOMEM;

    x = epoch * 4711 + cid * 33 + start_time;
    for (i = 0; i < tk->length; i++) {
	x += i * 3 + ((unsigned char *)k0->data)[i%k0->length];
	((unsigned char *)tk->data)[i] = 0x23 + x * 47;
    }

#if DEBUG
    print_key("k0: ", k0);
    print_key("tk: ", tk);
#endif

    return 0;
}

#endif /* AFS_RCRYPTO */
