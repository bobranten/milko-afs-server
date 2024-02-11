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

#include "rxgk_locl.h"

RCSID("$Id: rxgk_ticket.c,v 1.5 2007/05/14 21:30:03 lha Exp $");

#include <errno.h>

#include "rxgk_proto.h"

int
rxgk_encrypt_ticket(struct rxgk_ticket *ticket, RXGK_Ticket_Crypt *opaque)
{
    int ret;
    char *buf;
    size_t len;
    RXGK_Token clear, crypt;
    struct rxgk_keyblock key;

    len = RXGK_TICKET_MAX_SIZE;
    buf = malloc(RXGK_TICKET_MAX_SIZE);

    if (ydr_encode_rxgk_ticket(ticket, buf, &len) == NULL) {
	return errno;
    }

    clear.val = buf;
    clear.len = RXGK_TICKET_MAX_SIZE - len;

#if DEBUG
    fprintf(stderr, "before encrypt:");
    print_chararray(clear.val, clear.len);
#endif

    ret = rxgk_get_server_ticket_key(&key);
    if (ret) {
	free(buf);
	return EINVAL;
    }

    ret = rxgk_encrypt_buffer(&clear, &crypt, &key, RXGK_SERVER_ENC_TICKET);
    if (ret) {
	free(key.data);
	free(buf);
	return EINVAL;
    }

    opaque->val = crypt.val;
    opaque->len = crypt.len;
    
#if DEBUG
    fprintf(stderr, "after encrypt:");
    print_chararray(crypt.val, crypt.len);
#endif

    return 0;
}

int
rxgk_decrypt_ticket(RXGK_Ticket_Crypt *opaque, struct rxgk_ticket *ticket)
{
    size_t len;
    int ret;
    RXGK_Token clear, crypt;
    struct rxgk_keyblock key;

    ret = rxgk_get_server_ticket_key(&key);
    if (ret) {
	return EINVAL;
    }

    crypt.val = opaque->val;
    crypt.len = opaque->len;

#if DEBUG
    fprintf(stderr, "before decrypt:");
    print_chararray(crypt.val, crypt.len);
#endif

    ret = rxgk_decrypt_buffer(&crypt, &clear, &key, RXGK_SERVER_ENC_TICKET);
    if (ret) {
	free(key.data);
	return EINVAL;
    }

#if DEBUG
    fprintf(stderr, "after decrypt:");
    print_chararray(clear.val, clear.len);
#endif

    len = clear.len;
    if (ydr_decode_rxgk_ticket(ticket, clear.val, &len) == NULL) {
        return errno;
    }

    return 0;
}

int
rxgk_get_server_ticket_key(struct rxgk_keyblock *key)
{
    int i;
    /* XXX get real key */

    key->length = 32;
    key->enctype = RXGK_CRYPTO_AES256_CTS_HMAC_SHA1_96;
    
    key->data = malloc(key->length);
    if (key->data == NULL)
	return ENOMEM;

    for (i = 0; i < key->length; i++) {
	((unsigned char *)key->data)[i] = 0x23 + i * 47;
    }

    return 0;
}

