/*
 * Copyright (c) 2000 Kungliga Tekniska Högskolan
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

RCSID("$Id: kalog.c,v 1.13 2005/12/11 15:10:14 lha Exp $");

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <assert.h>

#include <ko.h>
#include <ports.h>
#include <log.h>

#include <arlalib.h>

#include <ka-procs.h>

#include <roken.h>
#include <getarg.h>
#include <err.h>

#include <vers.h>

static void
parse_user (char *argv1, const char **user, const char **cell)
{
    char *at = strchr(argv1, '@');
    char *tmp_cell;
    
    if(at) {
	*at = '\0';
	
	*user = argv1;
	at++;
	tmp_cell = at;
	
	if(*tmp_cell != '\0')
	    *cell = tmp_cell;
    } else {
	*user = argv1;
    }
    
}

static int num_hours = 8;
static int help_flag;
static int version_flag;

static struct getargs args[] = {
    {"hours",	  0,	arg_integer,	&num_hours,
     "hours to get tickets for", NULL},
    {"version",	0,	arg_flag,	&version_flag,
     NULL, NULL},
    {"help",	0,	arg_flag,	&help_flag,
     NULL, NULL}
};

static void
usage (int ret)
{
    arg_printusage (args, sizeof(args)/sizeof(*args), NULL, "[principal]");
    exit (ret);
}

int
main (int argc, char **argv)
{
    int ret, optidx = 0;
    Log_method *method;
    const char *cellname;
    const char *user;

    setprogname (argv[0]);
    tzset();

    method = log_open (getprogname(), "/dev/stderr:notime");
    if (method == NULL)
	errx (1, "log_open failed");
    cell_init(0, method);
    ports_init();
    
    cellname = cell_getthiscell();

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
	user = get_default_username();
    else if (argc == 1)
	parse_user (argv[0], &user, &cellname);
    else
	usage(1);

    printf ("Getting ticket for %s@%s\n", user, cellname);
    ret = ka_authenticate(user, "", cellname, NULL, num_hours * 3600);
    if (ret == EINVAL)
	errx(1, "kalog built without kaserver support");
    else if (ret)
	errx (1, "ka_authenticate failed with %s (%d)",
	      koerr_gettext(ret), ret);
    
    return 0;
}
