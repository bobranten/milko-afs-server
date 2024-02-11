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

RCSID("$Id: rxgk-settoken.c,v 1.4 2007/05/16 20:30:28 lha Exp $");

#include "roken.h"

#include <ko.h>
#include <kafs.h>
#include <token.h>
#include <getarg.h>

static int version_flag;
static int help_flag;
static char *cell_name;


static void
rxgk_set_token(const char *target, const char *cell)
{
    struct arlaViceIoctl vi;
    RXGK_Ticket_Crypt ticket;
    struct rxgk_keyblock key;
    uint32_t addr;
    time_t expire;
    rxgk_level level;
    int32_t flags;
    gss_name_t target_name = GSS_C_NO_NAME;
    int port, ret;
    token_opaque to;

    memset(&vi, 0, sizeof(vi));

    port = htons(7013);

    {
	OM_uint32 major_status, minor_status;
	gss_buffer_desc n;

	n.value = rk_UNCONST(target);
	n.length = strlen(target);
	
	major_status = gss_import_name(&minor_status, &n,
				       GSS_KRB5_NT_PRINCIPAL_NAME,
				       &target_name);
	if (GSS_ERROR(major_status))
	    err(1, "import name creds failed with: %d", major_status);
    }

    {
	const cell_db_entry *db_servers;
	int num_db_servers, i;
	int32_t cellnum;
	
	cellnum = cell_name2num(cell_name);
	if (cellnum < 0)
	    errx(1, "no such cell?");

	db_servers = cell_dbservers_by_id (cellnum, &num_db_servers);
	if (db_servers == NULL || num_db_servers == 0)
	    errx(1, "no db servers found for cell %s", cell_name);

	for (i = 0; i < num_db_servers; ++i) {
	    
	    addr = db_servers[i].addr.s_addr;

	    printf("server %s\n", inet_ntoa (db_servers[i].addr));

	    ret = rxgk_get_gss_cred(addr,
				    port,
				    GSS_C_NO_NAME, /* client */
				    target_name,
				    0, /* name tag */
				    &flags,
				    &expire,
				    &level,
				    &ticket,
				    &key);
	    if (ret == 0)
		break;
	    warnx("rxgk_get_gss_cred: %d", ret);
	}
	if (i == num_db_servers)
	    errx(1, "no dbserver happy");
    }

    {
	token_afs at;
	char *ptr, *rptr;
	size_t sz;
	
	memset(&at, 0, sizeof(at));

	at.at_type = 4;
	at.u.at_gk.gk_flags = flags;
	at.u.at_gk.gk_viceid = 0;
	at.u.at_gk.gk_begintime = 0;
	at.u.at_gk.gk_endtime = expire;
	at.u.at_gk.gk_level = level;
	at.u.at_gk.gk_lifetime = 0;
	at.u.at_gk.gk_bytelife = 0;
	at.u.at_gk.gk_enctype = key.enctype;
	at.u.at_gk.gk_key.len = key.length;
	at.u.at_gk.gk_key.val = key.data;
	at.u.at_gk.gk_token.len = ticket.len;
	at.u.at_gk.gk_token.val = ticket.val;

	sz = TOKEN_AFS_MAX_SIZE;
	ptr = emalloc(sz);

	rptr = ydr_encode_token_afs(&at, ptr, &sz);
	if (rptr == NULL)
	    errx(1, "foo");

	to.len = TOKEN_AFS_MAX_SIZE - sz;
	to.val = ptr;
    }

    {
	pioctl_set_token p;
	char *ptr, *rptr;
	size_t sz;

	memset(&p, 0, sizeof(p));
	p.flags = 0;
	strlcpy(p.cell, cell, sizeof(p.cell));
	p.tokens.len = 1;
	p.tokens.val = emalloc(sizeof(p.tokens.val[0]));

	p.tokens.val[0] = to;

	sz = PIOCTL_SET_TOKEN_MAX_SIZE;
	ptr = malloc(sz);
	
	rptr = ydr_encode_pioctl_set_token(&p, ptr, &sz);
	if (rptr == NULL)
	    errx(1, "foo");

	printf("pioctl_set_token size %d\n",
	       (int)(PIOCTL_SET_TOKEN_MAX_SIZE - sz));

	vi.in = ptr;
	vi.in_size = (int)(PIOCTL_SET_TOKEN_MAX_SIZE - sz);
    }

    ret = k_pioctl(NULL, ARLA_VIOCSETTOK2, (void *)&vi, 0);
    if (ret)
	err(1, "VIOCSETTOK2");
}


static struct getargs args[] = {
    {"cell", 'c',	arg_string,	&cell_name,
     "cell name",	"call"},
    {"version",	0,	arg_flag,	&version_flag,
     NULL, NULL},
    {"help",	0,	arg_flag,	&help_flag,
     NULL, NULL}
};

static void
usage (int ret)
{
    arg_printusage (args, sizeof(args)/sizeof(*args), NULL, "[device]");
    exit (ret);
}

int
main (int argc, char **argv)
{
    char *target, *realm_name;
    int optind = 0;

    setprogname (argv[0]);
    srand(time(NULL));

    if (getarg (args, sizeof(args)/sizeof(*args), argc, argv, &optind))
	usage (1);

    argc -= optind;
    argv += optind;

    if (help_flag)
	usage (0);

    if (version_flag) {
	print_version (NULL);
	exit (0);
    }
#if 0
    if (!k_hasafs ())
	errx(1, "no AFS");

    {
	Log_method *method;

	method = log_open(getprogname(), "/dev/stderr");
	if (method == NULL)
	    errx (1, "log_open failed");
	cell_init(0, method);
    }
#endif
    rx_Init(0);

    if (cell_name == NULL)
	cell_name = estrdup(cell_getthiscell());

    target = argv[1];

    realm_name = strdup(cell_name);
    strupr(realm_name);

    asprintf(&target, "rxgk/_afs.%s@%s", cell_name, realm_name);
    printf("target: %s\n", target);

    rxgk_set_token(target, cell_name);

    return 0;
}
