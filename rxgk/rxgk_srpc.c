/*
 * Copyright (c) 2002 - 2004, 2007, Stockholms universitet
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

RCSID("$Id: rxgk_srpc.c,v 1.10 2007/05/16 21:05:37 lha Exp $");

#include <errno.h>

#include <rx/rx.h>
#include "rxgk_proto.h"
#include "rxgk_proto.ss.h"

int
rxgk_make_ticket(struct rxgk_server_params *params, 
		 gss_ctx_id_t ctx, 
		 const void *snonce, size_t slength,
		 const void *cnonce, size_t clength,
		 RXGK_Ticket_Crypt *token, 
		 int32_t enctype)
{
    OM_uint32 major_status, minor_status;
    gss_buffer_desc exported_name;
    struct rxgk_keyblock key;
    struct rxgk_ticket ticket;
    gss_name_t src_name;
    OM_uint32 lifetime;
    int ret;

    memset(&ticket, 0, sizeof(ticket));

    ticket.ticketversion = 0;
    ticket.enctype = enctype;

    ret = rxgk_derive_k0(ctx, snonce, slength, cnonce, clength, enctype, &key);
    if (ret) {
	return ret;
    }
    ticket.key.len = key.length;
    ticket.key.val = key.data;
    ticket.level = RXGK_WIRE_ENCRYPT;
    ticket.starttime = time(NULL);
    major_status = gss_inquire_context(&minor_status,
				       ctx,
				       &src_name,
				       NULL,
				       &lifetime,
				       NULL, NULL, NULL, NULL);
    if (GSS_ERROR(major_status)) {
	_rxgk_gssapi_err(major_status, minor_status, GSS_C_NO_OID);
	free(ticket.key.val);
	return EINVAL;
    }

    ticket.expirationtime = time(NULL) + lifetime;

    ticket.lifetime = params->connection_lifetime;
    ticket.bytelife = params->bytelife;

    major_status = gss_export_name(&minor_status,
				   src_name,
				   &exported_name);
    if (GSS_ERROR(major_status)) {
	_rxgk_gssapi_err(major_status, minor_status, GSS_C_NO_OID);
	major_status = gss_release_name(&minor_status,
					&src_name);
	if (GSS_ERROR(major_status)) {
	    _rxgk_gssapi_err(major_status, minor_status, GSS_C_NO_OID);
	}
	free(ticket.key.val);
	return EINVAL;
    }

    major_status = gss_release_name(&minor_status,
				    &src_name);
    if (GSS_ERROR(major_status)) {
	_rxgk_gssapi_err(major_status, minor_status, GSS_C_NO_OID);
	major_status = gss_release_buffer(&minor_status, &exported_name);
	if (GSS_ERROR(major_status)) {
            _rxgk_gssapi_err(major_status, minor_status, GSS_C_NO_OID);
        }
	free(ticket.key.val);
	return EINVAL;
    }

    ticket.ticketprincipal.len = exported_name.length;
    ticket.ticketprincipal.val = malloc(exported_name.length);
    if (ticket.ticketprincipal.val == NULL) {
	major_status = gss_release_buffer(&minor_status, &exported_name);
	if (GSS_ERROR(major_status)) {
            _rxgk_gssapi_err(major_status, minor_status, GSS_C_NO_OID);
        }
	free(ticket.key.val);
        return EINVAL;
    }
    memcpy(ticket.ticketprincipal.val,
	   exported_name.value,
	   exported_name.length);
    
    major_status = gss_release_buffer(&minor_status, &exported_name);
    if (GSS_ERROR(major_status)) {
	_rxgk_gssapi_err(major_status, minor_status, GSS_C_NO_OID);
	free(ticket.key.val);
	return EINVAL;
    }

    ticket.ext.len = 0;
    ticket.ext.val = NULL;

    rxgk_encrypt_ticket(&ticket, token);

    /* free ticket */

    return 0;
}

static int
encrypt_clientinfo(struct RXGK_ClientInfo *clientinfo, RXGK_Token *opaque,
		   gss_ctx_id_t ctx)
{
    char *ret;
    size_t len;
    gss_buffer_desc buffer;
    gss_buffer_desc encrypted_buffer;
    OM_uint32 major_status, minor_status;

    len = RXGK_CLIENTINFO_MAX_SIZE;
    buffer.value = malloc(len);

    ret = ydr_encode_RXGK_ClientInfo(clientinfo, buffer.value, &len);
    if (ret == NULL) {
	return errno;
    }

    buffer.length = RXGK_CLIENTINFO_MAX_SIZE - len;

    major_status = gss_wrap(&minor_status,
			    ctx,
			    1,
			    GSS_C_QOP_DEFAULT,
			    &buffer,
			    NULL,
			    &encrypted_buffer);
    free(buffer.value);
    if (GSS_ERROR(major_status)) {
	_rxgk_gssapi_err(major_status, minor_status, GSS_C_NO_OID);
	return EINVAL;
    }

    opaque->val = malloc(encrypted_buffer.length);
    opaque->len = encrypted_buffer.length;
    memcpy(opaque->val, encrypted_buffer.value, encrypted_buffer.length);
    
    major_status = gss_release_buffer(&minor_status, &encrypted_buffer);
    if (GSS_ERROR(major_status)) {
	_rxgk_gssapi_err(major_status, minor_status, GSS_C_NO_OID);
	free(opaque->val);
	return EINVAL;
    }

    return 0;

}

static int
choose_enctype(const RXGK_Enctypes *client_enctypes,
	       const RXGK_Enctypes *server_enctypes,
	       int32_t *chosen_enctype) {

    if (client_enctypes->len == 0) {
	return EINVAL;
    }

    /* XXX actually do matching */
    *chosen_enctype = RXGK_CRYPTO_AES256_CTS_HMAC_SHA1_96;
    return 0;
}

int
SRXGK_GSSNegotiate(struct rx_call *call,
		   const struct RXGK_client_start *client_start,
		   const RXGK_Token *input_token_buffer,
		   const RXGK_Token *opaque_in,
		   RXGK_Token *output_token_buffer,
		   RXGK_Token *opaque_out,
		   uint32_t *gss_status,
		   RXGK_Token *rxgk_info)
{
    gss_buffer_desc input_token, output_token;
    OM_uint32 major_status, minor_status;
    gss_ctx_id_t ctx;
    struct rxgk_server_params *params;
    char servernonce[RXGK_MAX_NONCE];
    int retval;

    output_token_buffer->val = NULL;
    output_token_buffer->len = 0;

    opaque_out->val = NULL;
    opaque_out->len = 0;
    
    rxgk_info->val = NULL;
    rxgk_info->len = 0;

    /* the GSS-API context is stored on the server rock */
    ctx = rx_getConnRock(call->conn);
    params = (struct rxgk_server_params *) rx_getServiceRock(call->conn->service);

    input_token.value = input_token_buffer->val;
    input_token.length = input_token_buffer->len;

    major_status = gss_accept_sec_context(&minor_status,
					  &ctx,
					  GSS_C_NO_CREDENTIAL,
					  &input_token,
					  GSS_C_NO_CHANNEL_BINDINGS,
					  NULL,
					  NULL,
					  &output_token,
					  NULL,
					  NULL,
					  NULL);
    *gss_status = major_status;
    if (GSS_ERROR(major_status)) {
	_rxgk_gssapi_err(major_status, minor_status, GSS_C_NO_OID);
	gss_delete_sec_context(&minor_status, &ctx, NULL);
	goto out;
    }

    /* copy out the buffer */
    output_token_buffer->val = malloc(output_token.length);
    if (output_token_buffer->val == NULL) {
	gss_release_buffer(&minor_status, &output_token);
	goto out;
    }
    output_token_buffer->len = output_token.length;
    memcpy(output_token_buffer->val,
	   output_token.value, output_token.length);
    gss_release_buffer(&minor_status, &output_token);

    {
	RXGK_Ticket_Crypt ticket;
	struct RXGK_ClientInfo clientinfo;
	RXGK_Token encrypted_info;
	int32_t chosen_enctype;
	
	retval = choose_enctype(&client_start->sp_enctypes,
				&params->enctypes, &chosen_enctype);
	if (retval) {
	    gss_delete_sec_context(&minor_status, &ctx, NULL);
	    goto out;
	}
	
	memset(&clientinfo, 0, sizeof(clientinfo));
	
	clientinfo.ci_flags = params->flags;
	clientinfo.ci_level = params->level; /* XXX choose from client_start */

	clientinfo.ci_server_nonce.val = servernonce;
	clientinfo.ci_server_nonce.len = sizeof(servernonce);
	
	retval = rxgk_make_ticket(params, ctx, 
				  clientinfo.ci_server_nonce.val, 
				  clientinfo.ci_server_nonce.len,
				  client_start->sp_client_nonce.val,
				  client_start->sp_client_nonce.len,
				  &ticket, chosen_enctype);
	if (retval) {
	    gss_delete_sec_context(&minor_status, &ctx, NULL);
	    goto out;
	}
	
	clientinfo.ci_ticket = ticket;

	retval = encrypt_clientinfo(&clientinfo, &encrypted_info, ctx);
	if (retval) {
	    free(ticket.val);
	    gss_delete_sec_context(&minor_status, &ctx, NULL);
	    goto out;
	}

	*rxgk_info = encrypted_info;
    }
    
    gss_delete_sec_context(&minor_status, &ctx, NULL);

out:
    rx_setConnRock(call->conn, ctx);

    return retval;
}
