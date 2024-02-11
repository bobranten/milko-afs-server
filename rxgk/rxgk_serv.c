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

RCSID("$Id: rxgk_serv.c,v 1.15 2007/05/16 20:37:50 lha Exp $");

#include <errno.h>

/* Security object specific server data */
typedef struct rxgk_serv_class {
    struct rx_securityClass klass;
    rxgk_level min_level;
    char *service_name;

    int (*get_key)(void *, const char *, int, int, krb5_keyblock *);
    int (*user_ok)(const char *name, const char *realm, int kvno);
    int32_t (*rxgkService)(struct rx_call *);
    uint32_t serviceId;

    struct rxgk_server_params params;
} rxgk_serv_class;

/* Per connection specific server data */
typedef struct serv_con_data {
    end_stuff e;
    key_stuff k;
    uint64_t expires;
    char nonce[20];
    char authenticated;
    struct rxgk_ticket ticket;
    struct rxgk_keyblock tk;
} serv_con_data;

static int
server_NewConnection(struct rx_securityClass *obj, struct rx_connection *con)
{
    /*serv_con_data *cdat;*/
    assert(con->securityData == 0);
    obj->refCount++;
    con->securityData = (char *) osi_Alloc(sizeof(serv_con_data));
    memset(con->securityData, 0x0, sizeof(serv_con_data));
    /*cdat = (serv_con_data *)con->securityData;*/
    return 0;
}

static int
server_Close(struct rx_securityClass *obj)
{
    obj->refCount--;
    if (obj->refCount <= 0)
	osi_Free(obj, sizeof(rxgk_serv_class));
    return 0;
}

static
int
server_DestroyConnection(struct rx_securityClass *obj,
			 struct rx_connection *con)
{
  serv_con_data *cdat = (serv_con_data *)con->securityData;

  if (cdat)
      osi_Free(cdat, sizeof(serv_con_data));
  return server_Close(obj);
}

/*
 * Check whether a connection authenticated properly.
 * Zero is good (authentication succeeded).
 */
static int
server_CheckAuthentication(struct rx_securityClass *obj,
			   struct rx_connection *con)
{
    serv_con_data *cdat = (serv_con_data *) con->securityData;

    if (cdat)
	return !cdat->authenticated;
    else
	return RXGKNOAUTH;
}

/*
 * Select a nonce for later use.
 */
static
int
server_CreateChallenge(struct rx_securityClass *obj_,
		       struct rx_connection *con)
{
    serv_con_data *cdat = (serv_con_data *) con->securityData;
    int i;

    for (i = 0; i < sizeof(cdat->nonce)/sizeof(cdat->nonce[0]); i++)
	cdat->nonce[i] = 17; /* XXX */
    cdat->authenticated = 0;
    return 0;
}

/*
 * Wrap the nonce in a challenge packet.
 */
static int
server_GetChallenge(const struct rx_securityClass *obj_,
		    const struct rx_connection *con,
		    struct rx_packet *pkt)
{
    serv_con_data *cdat = (serv_con_data *) con->securityData;
    struct RXGK_Challenge c;

    c.rc_version = htonl(RXGK_VERSION);
    memcpy(c.rc_nonce, cdat->nonce, sizeof(c.rc_nonce));

    /* Stuff into packet */
    if (rx_SlowWritePacket(pkt, 0, sizeof(c), (void*)&c) != sizeof(c))
	return RXGKPACKETSHORT;
    rx_SetDataSize(pkt, sizeof(c));

    return 0;
}

/*
 * Process a response to a challange.
 */
static int
server_CheckResponse(struct rx_securityClass *obj_,
		     struct rx_connection *con,
		     struct rx_packet *pkt)
{
    struct rxgk_serv_class *serv_class = 
	(struct rxgk_serv_class *)obj_->privateData;
    serv_con_data *cdat = (serv_con_data *) con->securityData;
    int ret;
    struct RXGK_Response_Crypt rc;
    struct RXGK_Response r;
    char response[RXGK_RESPONSE_MAX_SIZE];
    size_t len, len2;
    RXGK_Token rc_clear, rc_crypt;
    struct rxgk_keyblock k0;
    const char *p;
    int i;

    memset(&r, 0, sizeof(r));
    memset(&rc, 0, sizeof(rc));
    
    len = rx_SlowReadPacket(pkt, 0, sizeof(response), response);
    if (len <= 0)
	return RXGKPACKETSHORT;
    
    len2 = len;
    if (ydr_decode_RXGK_Response(&r, response, &len2) == NULL) {
	ret = RXGKPACKETSHORT;
	goto out;
    }

    ret = rxgk_decrypt_ticket(&r.rr_authenticator, &cdat->ticket);
    if (ret) {
	ret = RXGKPACKETSHORT;
	goto out;
    }

    k0.data = cdat->ticket.key.val;
    k0.length = cdat->ticket.key.len;
    k0.enctype = cdat->ticket.enctype;

    ret = rxgk_derive_transport_key(&k0, &cdat->tk, con->epoch, 
				    con->cid, r.start_time);
    if (ret) {
	return ret;
    }

    rc_crypt.val = r.rr_ctext.val;
    rc_crypt.len = r.rr_ctext.len;

    ret = rxgk_decrypt_buffer(&rc_crypt, &rc_clear,
			      &cdat->tk, RXGK_CLIENT_ENC_RESPONSE);
    if (ret) {
	free(cdat->tk.data);
	ret = RXGKPACKETSHORT;
	goto out;
    }

    len = rc_clear.len;
    p = ydr_decode_RXGK_Response_Crypt(&rc, rc_clear.val, &len);
    free(rc_clear.val);
    if (p == NULL)
	return RXGKPACKETSHORT;

    if (rc.epoch != con->epoch ||
	rc.cid != (con->cid & RX_CIDMASK)) {
	return RXGKPACKETSHORT;
    }
    if (memcmp(rc.nonce, cdat->nonce, sizeof(rc.nonce)) != 0) {
	return RXGKSEALEDINCON;
    }

    if (cdat->ticket.level < serv_class->min_level)
	return RXGKLEVELFAIL;
    if (cdat->ticket.level != RXGK_WIRE_ENCRYPT) /* XXX */
	return RXGKLEVELFAIL;

    for (i = 0; i < RX_MAXCALLS; i++) {
	if (rc.call_numbers[i] < 0) {
	    ret = RXGKSEALEDINCON;
	    goto out;
	}
    }
    rxi_SetCallNumberVector(con, rc.call_numbers);

    ret = rxgk_crypto_init(&cdat->tk, &cdat->k);
    if (ret)
	goto out;

    if (cdat->ticket.level == RXGK_WIRE_ENCRYPT) {
	rx_SetSecurityHeaderSize(con, 12);
	rx_SetSecurityMaxTrailerSize(con, cdat->k.ks_overhead + 8);
    } else {
	ret = RXGKLEVELFAIL;
	goto out;
    }

    cdat->authenticated = 1;

 out:  
    ydr_free_RXGK_Response(&r);
    ydr_free_RXGK_Response_Crypt(&rc);

    return ret;
}

/*
 * Checksum and/or encrypt packet
 */
static int
server_PreparePacket(struct rx_securityClass *obj_,
		     struct rx_call *call,
		     struct rx_packet *pkt)
{
    struct rx_connection *con = rx_ConnectionOf(call);
    serv_con_data *cdat = (serv_con_data *) con->securityData;

    return rxgk_prepare_packet(pkt, con, cdat->ticket.level,
			       &cdat->k, &cdat->e,
			       RXGK_SERVER_ENC_PACKET,
			       RXGK_SERVER_MIC_PACKET);
}

/*
 * Verify checksum and/or decrypt packet.
 */
static int
server_CheckPacket(struct rx_securityClass *obj_,
		   struct rx_call *call,
		   struct rx_packet *pkt)
{
    struct rx_connection *con = rx_ConnectionOf(call);
    serv_con_data *cdat = (serv_con_data *) con->securityData;

    /* Use fast time package instead??? */
    if (time(0) > cdat->ticket.expirationtime)
	return RXGKEXPIRED;

    return rxgk_check_packet(pkt, con, cdat->ticket.level,
			     &cdat->k, &cdat->e,
			     RXGK_CLIENT_ENC_PACKET,
			     RXGK_CLIENT_MIC_PACKET);
}

static int
server_GetStats(const struct rx_securityClass *obj_,
		const struct rx_connection *con,
		struct rx_securityObjectStats *st)
{
    rxgk_serv_class *obj = (rxgk_serv_class *) obj_;
    serv_con_data *cdat = (serv_con_data *) con->securityData;
    
    st->type = rxgk_disipline;
    st->level = obj->min_level;
    st->flags = rxgk_checksummed;
    if (cdat == 0)
	st->flags |= rxgk_unallocated;
    {
	st->bytesReceived = cdat->e.bytesReceived;
	st->packetsReceived = cdat->e.packetsReceived;
	st->bytesSent = cdat->e.bytesSent;
	st->packetsSent = cdat->e.packetsSent;
	st->expires = cdat->ticket.expirationtime;
	st->level = cdat->ticket.level;
	if (cdat->authenticated)
	    st->flags |= rxgk_authenticated;
    }
    return 0;
}

static
void
free_context(struct rx_connection * conn)
{
    return;
}

static
int
server_NewService(const struct rx_securityClass *obj_,
		  struct rx_service *service,
		  int reuse)
{
    struct rxgk_serv_class *serv_class = (struct rxgk_serv_class *)obj_->privateData;

    if (service->serviceId == RXGK_SERVICE_ID)
	return 0;
    
    if (!reuse && serv_class->rxgkService) {
	struct rx_securityClass *sec[2];
	struct rx_service *secservice;
	
	sec[0] = rxnull_NewServerSecurityObject();
	sec[1] = NULL;
	
	secservice = rx_NewService (service->servicePort,
				    RXGK_SERVICE_ID,
				    "rxgk", 
				    sec, 1, 
				    serv_class->rxgkService);
	
	secservice->destroyConnProc = free_context;
	rx_setServiceRock(secservice, &serv_class->params);
    }
    return 0;
}


static struct rx_securityOps server_ops = {
    server_Close,
    server_NewConnection,
    server_PreparePacket,
    NULL,
    server_CheckAuthentication,
    server_CreateChallenge,
    server_GetChallenge,
    NULL,
    server_CheckResponse,
    server_CheckPacket,
    server_DestroyConnection,
    server_GetStats,
    server_NewService,
};

struct rx_securityClass *
rxgk_NewServerSecurityObject(rxgk_level min_level,
			     const char *gss_service_name,
			     int32_t (*rxgkService)(struct rx_call *),
			     struct rxgk_server_params *params)
{
    rxgk_serv_class *obj;

    rxgk_crypto_start();

    if ((gss_service_name == NULL) ^ (rxgkService == NULL))
	return NULL;

    obj = (rxgk_serv_class *) osi_Alloc(sizeof(rxgk_serv_class));
    obj->klass.refCount = 1;
    obj->klass.ops = &server_ops;
    obj->klass.privateData = (char *) obj;
    obj->params = *params;
    obj->min_level = min_level;

    if (rxgkService) {
	obj->service_name = strdup(gss_service_name);
	if (obj->service_name == NULL) {
	    osi_Free(obj, sizeof(*obj));
	    return NULL;
	}
	obj->rxgkService = rxgkService;
    }
    
    return &obj->klass;
}

static const unsigned char k5oid[9] = "\x2a\x86\x48\x86\xf7\x12\x01\x02\x02";

int32_t
rxgk_GetServerInfoOld(struct rx_connection * con, rxgk_level * level,
		      uint32_t * expiration, char **rname, char **rcell)
{
    serv_con_data *cdat = (serv_con_data *) con->securityData;
    uint32_t length;
    char *p, *name, *realm;
    
    if (level)
	*level = cdat->ticket.level;
    if (expiration)
	*expiration = cdat->ticket.expirationtime;

    if (cdat->ticket.ticketprincipal.len < 10 + sizeof(k5oid))
	return RXGKNOAUTH;

    /* TOK, MECH_OID_LEN, DER(MECH_OID), NAME_LEN, NAME */
    p = cdat->ticket.ticketprincipal.val;

    if (memcmp(&p[0], "\x04\x01\x00", 3) != 0 ||
	p[3] != sizeof(k5oid) + 2 ||
	p[4] != 0x06 ||
	p[5] != sizeof(k5oid) ||
	memcmp(&p[6], k5oid, sizeof(k5oid)) != 0)
	return RXGKNOAUTH;

    p += 6 + sizeof(k5oid);

    length = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    p += 4;

    if (length > cdat->ticket.ticketprincipal.len - 10 - sizeof(k5oid))
	return RXGKNOAUTH;

    name = malloc(length + 1);
    if (name == NULL)
	return RXGKNOAUTH;
    memcpy(name, p, length);
    name[length] = '\0';

    realm = strchr(name, '@');
    if (realm == NULL) {
	free(name);
	return RXGKNOAUTH;
    }
    *realm++ = '\0';

    if (rcell) {
	*rcell = strdup(realm);
	if (*rcell == NULL) {
	    free(name);
	    return RXGKNOAUTH;
	}
    }
    if (rname)
	*rname = name;
    else
	free(name);
    
    return 0;
}

void
rxgk_freeInfoOld(char *str)
{
    free(str);
}
