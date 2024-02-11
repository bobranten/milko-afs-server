/*
 * Copyright (c) 2007 Kungliga Tekniska Högskolan
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

#include <config.h>

#include <gssapi.h>
#include <rx/rx.h>
#include <rxgk/rxgk_proto.h>
#include <rxgk/rxgk.h>

#include <arla-pioctl.h>
#include <kafs.h>

#include <getarg.h>
#include <roken.h>
#include <vers.h>

RCSID("$Id: rxgklog.c,v 1.3 2007/01/25 10:43:35 lha Exp $");

int
rxgk_get_gss_cred(uint32_t, short, gss_name_t, gss_name_t, time_t*,
                  int32_t *, time_t *, rxgk_level *,
                  RXGK_Ticket_Crypt *, struct rxgk_keyblock *);

static int
rxgklog(const char *cell)
{
    gss_name_t target_name;
    uint32_t host = 0;
    time_t expire;
    rxgk_level level;
    int32_t nametag = 0;
    RXGK_Ticket_Crypt ticket;
    struct rxgk_keyblock key;
    struct arlaViceIoctl parms;
    char *buf, *t;
    int32_t sizeof_x;
    int64_t sizeof_xx;
    int ret;
    

    {
	char *target;
	OM_uint32 major_status, minor_status;
	gss_buffer_desc n;

	asprintf(&target, "afsgk@_afs.%s", cell);
	if (target == NULL)
	    errx(1, "out of memory");

	n.value = target;
	n.length = strlen(target);
	
	major_status = gss_import_name(&minor_status, &n,
				       GSS_C_NT_HOSTBASED_SERVICE,
				       &target_name);
	free(target);
	if (GSS_ERROR(major_status))
	    err(1, "import name creds failed with: %d", major_status);
    }


    /* fetching rxgk token */
    ret = rxgk_get_gss_cred(host, 7001, NULL, target_name, 
			    &expire, &nametag , &expire, &level, &ticket, &key);
    if (ret)
	errx(1, "rxgk_get_gss_cred");


    buf = emalloc(ticket.len + key.length + 2048);

    t = buf;

    /* flags */
    sizeof_x = 0;
    memcpy(t, &sizeof_x, sizeof(sizeof_x));
    t += sizeof(sizeof_x);
    /* viceid */
    sizeof_x = 0;
    memcpy(t, &sizeof_x, sizeof(sizeof_x));
    t += sizeof(sizeof_x);
    /* starttime */
    sizeof_xx = time(NULL) - 60;
    memcpy(t, &sizeof_xx, sizeof(sizeof_xx));
    t += sizeof(sizeof_xx);
    /* endtime */
    sizeof_xx = expire;
    memcpy(t, &sizeof_xx, sizeof(sizeof_xx));
    t += sizeof(sizeof_xx);
    /* celllength */
    sizeof_x = strlen(cell) + 1;
    memcpy(t, &sizeof_x, sizeof(sizeof_x));
    t += sizeof(sizeof_x);
    /* cell */
    memcpy(t, cell, sizeof_x);
    t += sizeof_x;
    /* num tokens */
    sizeof_x = 1;
    memcpy(t, &sizeof_x, sizeof(sizeof_x));
    t += sizeof(sizeof_x);
    /* token type */
    sizeof_x = ARLA_TOKEN_TYPE_GK;
    memcpy(t, cell, sizeof(sizeof_x));
    t += sizeof_x;
    /* token length */
    sizeof_x = 4 + 4 + 4 + 4 + 4 + ticket.len + 4 + key.length;
    memcpy(t, cell, sizeof(sizeof_x));
    t += sizeof_x;

    /* rxgk token */
    /* level */
    sizeof_x = RXGK_WIRE_ENCRYPT;
    memcpy(t, &sizeof_x, sizeof(sizeof_x));
    t += sizeof(sizeof_x);
    /* enctype */
    sizeof_x = key.enctype;
    memcpy(t, &sizeof_x, sizeof(sizeof_x));
    t += sizeof(sizeof_x);
    /* lifetime */
    sizeof_x = 0;
    memcpy(t, &sizeof_x, sizeof(sizeof_x));
    t += sizeof(sizeof_x);
    /* bytelife */
    sizeof_x = 0;
    memcpy(t, &sizeof_x, sizeof(sizeof_x));
    t += sizeof(sizeof_x);
    /* encrypted ticket length */
    sizeof_x = ticket.len;
    memcpy(t, &sizeof_x, sizeof(sizeof_x));
    t += sizeof(sizeof_x);
    /* encrypted token data */
    memcpy(t, ticket.val, ticket.len);
    t += ticket.len;
    /* key length */
    sizeof_x = key.length;
    memcpy(t, &sizeof_x, sizeof(sizeof_x));
    t += sizeof(sizeof_x);
    /* key data */
    memcpy(t, key.data, sizeof_x);
    t += key.length;

    /*
     * Build argument block
     */
    parms.in = buf;
    parms.in_size = t - buf;
    parms.out = 0;
    parms.out_size = 0;

    ret = k_pioctl(0, ARLA_VIOCSETTOK2, (void *)&parms, 0);
    free(buf);
    if (ret)
	errx(1, "k_pioctl");
    

    return 0;
}




static int version_flag;
static int help_flag;

static struct getargs args[] = {
    {"version",	0,	arg_flag,	&version_flag,
     NULL, NULL},
    {"help",	0,	arg_flag,	&help_flag,
     NULL, NULL}
};

static void
usage (int ret)
{
    arg_printusage (args, sizeof(args)/sizeof(*args), NULL, "cell ...");
    exit (ret);
}

int
main (int argc, char **argv)
{
    int optidx = 0;
    int i, ret;

    setprogname (argv[0]);

    if (getarg (args, sizeof(args)/sizeof(*args), argc, argv, &optidx))
	usage (1);

    argc -= optidx;
    argv += optidx;

    if (help_flag)
	usage (0);

    if (version_flag) {
	print_version (NULL);
	exit (0);
    }
    
    if (argc == 0)
	usage(1);

    rx_Init(0);

    for (i = 0; i < argc; argc++) {
	ret = rxgklog(argv[i]);
	if (ret)
	    errx(1, "rxgklog");
    }

    return 0;
}
