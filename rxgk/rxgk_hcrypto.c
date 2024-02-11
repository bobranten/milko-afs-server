/*
 * Copyright (c) 2002 - 2007, Stockholms universitet
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

RCSID("$Id: rxgk_hcrypto.c,v 1.15 2007/05/16 18:31:27 lha Exp $");

#ifdef AFS_HCRYPTO

krb5_context _rxgkk5ctx;

void
rxgk_crypto_start(void)
{
    if (_rxgkk5ctx == NULL)
	krb5_init_context(&_rxgkk5ctx);
}

int
rxgk_crypto_init(struct rxgk_keyblock *tk, key_stuff *k)
{
    krb5_keyblock *tk_kb;
    int ret;

    ret = krb5_init_keyblock(_rxgkk5ctx, tk->enctype, tk->length,
			     &tk_kb);

    if (ret)
	return EINVAL;

    /*ret = krb5_crypto_init (_rxgkk5ctx, &tk_kb, tk->enctype, &k->ks_scrypto);*/
    krb5_free_keyblock_contents(_rxgkk5ctx, tk_kb);
    if (ret)
	return EINVAL;

    krb5_c_block_size(_rxgkk5ctx, k->ks_scrypto, &k->ks_overhead);

    {
	krb5_cksumtype *types;
	unsigned int count;
	ret = krb5_c_keyed_checksum_types(_rxgkk5ctx, k->ks_scrypto, &count, &types);
	ret = krb5_c_checksum_length(_rxgkk5ctx, types[1], &k->ks_cksumsize); 
	krb5_free_cksumtypes(_rxgkk5ctx, types);
    }

    return 0;
}

struct rxgk_pkg_hdr {
    uint32_t call_number;
    uint32_t channel_and_seq;
    uint32_t svcid_len;
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
}

int
rxgk_prepare_packet(struct rx_packet *p, struct rx_connection *conn,
		    int level, key_stuff *k, end_stuff *e,
		    int keyusage_enc, int keyusage_mic)
{
    size_t len = rx_GetDataSize(p);
    size_t off = rx_GetSecurityHeaderSize(conn);
    int ret;

    if (level == RXGK_WIRE_ENCRYPT) {
	krb5_data plain, cipher;
	struct rxgk_pkg_hdr hdr;

	memset(&cipher, 0, sizeof(cipher));

	assert(sizeof(hdr) == off);

	plain.length = len + sizeof(hdr);
	plain.data = osi_Alloc(plain.length);

	getheader(conn, p, &hdr);
	hdr.svcid_len = htonl((conn->serviceId << 16) | rx_GetDataSize(p));
	memcpy(plain.data, &hdr, sizeof(hdr));

	rx_packetread(p, off, len, (unsigned char*)plain.data + sizeof(hdr));

	ret = krb5_encrypt(_rxgkk5ctx, k->ks_scrypto, keyusage_enc,
			   plain.data, plain.length, &cipher);
	osi_Free(plain.data, plain.length);

	if (ret == 0) {
	    if (cipher.length > len + sizeof(hdr))
		rxi_RoundUpPacket(p, cipher.length - len - sizeof(hdr));
	    rx_packetwrite(p, 0, cipher.length, cipher.data);
	    rx_SetDataSize(p, cipher.length);
	    /*krb5_data_free(&cipher);*/
	}
    } else if (level == RXGK_WIRE_INTEGRITY) {
	krb5_checksum cksum;
	krb5_data scratch;

	memset(&cksum, 0, sizeof(cksum));
	
	scratch.length = len;
	scratch.data = osi_Alloc(scratch.length);

	rx_packetread(p, off, len, scratch.data);

	ret = krb5_c_make_checksum(_rxgkk5ctx, k->ks_scrypto, keyusage_mic, 0,
				   &scratch, &cksum);
	osi_Free(scratch.data, scratch.length);
	rx_SetDataSize(p, len);

	/* write header into trailier */

    } else if (level == RXGK_WIRE_AUTH_ONLY) {
	ret = 0;
    } else {
	abort();
    }
    return ret;
}

/*
 *
 */
int
rxgk_check_packet(struct rx_packet *p, struct rx_connection *conn,
		  int level, key_stuff *k, end_stuff *e,
		  int keyusage_enc, int keyusage_mic)
{
    size_t len = rx_GetDataSize(p);
    size_t off = rx_GetSecurityHeaderSize(conn);
    int ret = 0;

    if (level == RXGK_WIRE_ENCRYPT) {
	krb5_data cipher, plain;
	struct rxgk_pkg_hdr hdr;

	plain.data = NULL;
	plain.length = 0;

	cipher.length = len;
	cipher.data = osi_Alloc(cipher.length);

	getheader(conn, p, &hdr);
	rx_packetread(p, 0, cipher.length, cipher.data);

	ret = krb5_decrypt(_rxgkk5ctx, k->ks_scrypto, keyusage_enc,
			    cipher.data, cipher.length, &plain);
	osi_Free(cipher.data, cipher.length);
	if (ret == 0) {
	    if (plain.length < sizeof(hdr)) {
		ret = EINVAL;
		goto out;
	    }
	    if (memcmp(plain.data, &hdr, 8) != 0) {
		ret = ENOENT;
		goto out;
	    }
	    len = ntohl(((uint32_t *)plain.data)[2]) & 0xffff;
	    if (len > plain.length - sizeof(hdr)) {
		ret = EIO;
		goto out;
	    }
	    rx_packetwrite(p, off, 
			   len, 
			   (unsigned char*)plain.data + sizeof(hdr));
	    rx_SetDataSize(p, len);
	out:
	    /*krb5_data_free(&plain);*/
	}

    } else if (level == RXGK_WIRE_INTEGRITY) {
	krb5_data scratch;
	krb5_checksum cksum;

	memset(&cksum, 0, sizeof(cksum));

	scratch.length = len;
	scratch.data = osi_Alloc(scratch.length);

	rx_Pullup(p, off);
	rx_packetread(p, off, len-off, scratch.data);
	scratch.length = len - off;

	/*ret = krb5_verify_checksum(_rxgkk5ctx, k->ks_scrypto, keyusage_mic,
				    scratch.data, scratch.length,
				    &cksum);*/
	osi_Free(scratch.data, scratch.length);
	if (ret == 0)
	    rx_SetDataSize(p, len - off);

    } else if (level == RXGK_WIRE_AUTH_ONLY) {
	ret = 0;
    } else {
	abort();
    }

    return ret;
}

int
rxgk_encrypt_buffer(RXGK_Token *in, RXGK_Token *out,
		    struct rxgk_keyblock *key, int keyusage)
{
    krb5_keyblock *keyblock;
    krb5_cryptotype crypto = 0;
    krb5_data data;
    int ret;

    ret = krb5_init_keyblock(_rxgkk5ctx, key->enctype,
			     key->length, &keyblock);
    if (ret)
	return EINVAL;

    /*ret = krb5_crypto_init(_rxgkk5ctx, &keyblock, 0, &crypto);*/
    krb5_free_keyblock_contents(_rxgkk5ctx, keyblock);
    if (ret)
	return EINVAL;

    ret = krb5_encrypt(_rxgkk5ctx, crypto, keyusage, in->val, in->len, &data);
    krb5_cc_destroy(_rxgkk5ctx, crypto);

    out->val = data.data;
    out->len = data.length;

    return ret;
}

int
rxgk_decrypt_buffer(RXGK_Token *in, RXGK_Token *out,
		    struct rxgk_keyblock *key, int keyusage)
{
    krb5_keyblock *keyblock;
    krb5_cryptotype crypto = 0;
    krb5_data data;
    int ret;

    ret = krb5_init_keyblock(_rxgkk5ctx, key->enctype,
			     key->length, &keyblock);
    if (ret)
	return EINVAL;

    /*ret = krb5_crypto_init(_rxgkk5ctx, &keyblock, 0, &crypto);*/
    krb5_free_keyblock_contents(_rxgkk5ctx, keyblock);
    if (ret)
	return EINVAL;

    ret = krb5_decrypt(_rxgkk5ctx, crypto, keyusage, in->val, in->len, &data);
    krb5_cc_destroy(_rxgkk5ctx, crypto);

    out->val = data.data;
    out->len = data.length;

    return ret;
}

#if DEBUG
static void
print_key(char *name, struct rxgk_keyblock *key)
{
    int i;

    fprintf(stderr, "type: %s", name);
    for (i = 0; i < key->length; i++)
	fprintf(stderr, " %02x", ((unsigned char*)key->data)[i]);
    fprintf(stderr, "\n");    
}
#endif

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

#endif /* AFS_HCRYPTO */
