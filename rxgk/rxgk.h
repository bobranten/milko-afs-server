/*
 * Copyright (c) 2004, Stockholms universitet
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

/* $Id: rxgk.h,v 1.8 2007/05/16 21:04:13 lha Exp $ */

#ifndef __RXGK_H
#define __RXGK_H

typedef int rxgk_level;

struct rxgk_ticket;

struct rxgk_keyblock {
    uint32_t enctype;
    size_t length;
    void *data;
};

int32_t
rxgk_GetServerInfo(struct rx_connection *con,
		   struct rxgk_ticket **ticket);

struct rxgk_server_params {
    int32_t connection_lifetime;
    int32_t bytelife;
    int32_t flags;
    int32_t level; /* min level */
    RXGK_Enctypes enctypes;
};

struct rx_securityClass *
rxgk_NewServerSecurityObject (rxgk_level min_level,
			      const char *gss_service_name,
			      int32_t (*rxgkService)(struct rx_call *),
			      struct rxgk_server_params *params);

struct rx_securityClass *
rxgk_NewClientSecurityObject (rxgk_level level,
			      RXGK_Ticket_Crypt *token, 
			      struct rxgk_keyblock *key);

typedef void (*rxgk_logf)(void *, const char *, ...);

void
rxgk_set_log(rxgk_logf, void *);

void
rxgk_log_stdio(void *, const char *, ...);

int32_t
rxgk_GetServerInfoOld(struct rx_connection *, rxgk_level * ,
		      uint32_t * , char **, char ** );

void rxgk_freeInfoOld(char *);

int32_t
RXGK_ExecuteRequest(struct rx_call *);

#endif /* __RXGK_H */
