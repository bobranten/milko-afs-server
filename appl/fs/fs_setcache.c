/*
 * Copyright (c) 1995 - 2001, 2005 Kungliga Tekniska Högskolan
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

#include "fs_local.h"

RCSID("$Id: fs_setcache.c,v 1.2 2005/10/28 14:33:34 tol Exp $");

static int
afs_setcache(int64_t highbytes, int64_t lowbytes, int64_t highvnodes, int64_t lowvnodes)
{
    int ret;

    ret = fs_setcacheparam(highbytes, lowbytes, highvnodes, lowvnodes);
    if (ret)
	fserr(PROGNAME, ret, ".");
    return ret;
}

int
setcache_cmd (int argc, char **argv)
{
    int highbytes = 0;
    int lowbytes = 0;
    int highvnodes = 0;
    int lowvnodes = 0;
    int helpflag = 0;
    int optind = 0;

    struct agetargs sqargs[] = {
	{"highbytes",  0, aarg_integer, NULL, "maximum cache size in kbytes",
	 "kbytes",   aarg_optional_swless},
	{"lowbytes",  0, aarg_integer, NULL, "target cache size in kbytes",
	 "kbytes",   aarg_optional_swless},
	{"highvnodes",  0, aarg_integer, NULL, "maximum number of nodes in cache",
	 NULL,   aarg_optional_swless},
	{"lowvnodes",  0, aarg_integer, NULL, "target number of nodes in cache",
	 NULL,   aarg_optional_swless},
	{"help",    0, aarg_flag, NULL },
        {NULL,      0, aarg_end, NULL}}, *arg;
    
    arg = sqargs;
    arg->value = &highbytes; arg++;
    arg->value = &lowbytes; arg++;
    arg->value = &highvnodes; arg++;
    arg->value = &lowvnodes; arg++;
    arg->value = &helpflag; arg++;

    if (agetarg (sqargs, argc, argv, &optind, AARG_AFSSTYLE)) {
	aarg_printusage(sqargs, "fs setcachesize", NULL, AARG_AFSSTYLE);
	return 0;
    }

    if (helpflag) {
	aarg_printusage(sqargs, "fs setcachesize", NULL, AARG_AFSSTYLE);
	return 0;
    }
    argc -= optind;
    argv += optind;

    if (argc) {
	printf("unknown option %s\n", *argv);
	return 0;
    }
    afs_setcache(highbytes * 1024LL, lowbytes * 1024LL, highvnodes, lowvnodes);

    return 0;
}
