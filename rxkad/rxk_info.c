/*
 * Copyright (c) 1995, 1996, 1997, 2003 Kungliga Tekniska Högskolan
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

RCSID("$Id: rxk_info.c,v 1.6 2003/11/13 05:48:34 lha Exp $");

/*
 * Return value of `name' depends on `con', its free()ed when con is
 * free()ed so don't do that.
 */

int32_t
rxkad_GetServerInfoNew(struct rx_connection *con,
		       rxkad_level *level,
		       uint32_t *expiration,
		       char **name,
		       int32_t *kvno)
{
  serv_con_data *cdat = (serv_con_data *) con->securityData;

  if (cdat && cdat->authenticated
      && (time(0) < cdat->expires)
      && cdat->user)
    {
      if (level)
	*level = cdat->cur_level;
      if (expiration)
	*expiration = cdat->expires;
      if (name)
	*name = cdat->user;
      if (kvno)
	*kvno = -1;		/* Where do we find this and who needs it? */
      return 0;
    }
  else
    return RXKADNOAUTH;
}
