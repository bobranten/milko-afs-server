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

#include <sys/socket.h>
#include <arpa/inet.h>

#include <err.h>
#include <netdb.h>

#include "rxgk_proto.ss.h"

#include "roken.h"

RCSID("$Id: rxgk-service.c,v 1.3 2007/06/28 18:28:17 map Exp $");

#define DEFAULT_PORT	7013

int mixed_cell = 1;

/*
 *
 */

int
main(int argc, char **argv)
{
    struct rxgk_server_params params;
    int port, ret;
    PROCESS pid;

    setprogname(argv[0]);

    port = htons(DEFAULT_PORT);

    LWP_InitializeProcessSupport (LWP_NORMAL_PRIORITY, &pid);

    rxgk_set_log(rxgk_log_stdio, NULL);

    ret = rx_Init (port);
    if (ret)
	errx (1, "rx_Init failed");

    memset(&params, 0, sizeof(params));

    params.connection_lifetime = 10 * 3600;
    params.bytelife = (uint64_t)10 * 1024 * 1024;
    params.enctypes.len = 0;
    params.enctypes.val = NULL;
    params.level = RXGK_WIRE_ENCRYPT;
    params.flags = 0;
    if (mixed_cell)
	params.flags |= RXGK_CI_FLAG_MIXED;


    {
	struct rx_securityClass *sec[2];
	struct rx_service *secservice;
	
	sec[0] = rxnull_NewServerSecurityObject();
	sec[1] = NULL;
	
	secservice = rx_NewService (0,
				    RXGK_SERVICE_ID,
				    "rxgk", 
				    sec, 1, 
				    RXGK_ExecuteRequest);
	if (secservice == NULL) 
	    errx(1, "Cant create server");

	rx_setServiceRock(secservice, &params);
    }
    rx_StartServer(1) ;

    return 0;
}
