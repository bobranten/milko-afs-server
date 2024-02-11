/*
 * Copyright (c) 1999,2000,2004 Kungliga Tekniska Högskolan
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

#include "appl_locl.h"
#include <sl.h>
#include "vos_local.h"

RCSID("$Id: vos_dump.c,v 1.11 2005/03/26 11:41:11 lha Exp $");

static void
dump_volume (const char *volume,
	     const char *cell, const char *host, const char *part,
	     const char *file, int32_t fromtime, arlalib_authflags_t auth,
	     int verbose)
{
    struct rx_connection *conn_volser = NULL;
    struct rx_call *call;
    int error;
    int fd;
    nvldbentry the_vlentry;
    int32_t trans_id;
    int ret;
    uint32_t vol_id;
    int have_volid = 0;
    uint32_t volser_addr;
    int part_id;
    uint32_t size, nread;
    char buf[8192];

    if (file != NULL) {
	fd = open (file, O_WRONLY | O_CREAT, 0666);
	if (fd < 0) {
	    warn ("open %s", file);
	    return;
	}
    } else {
	fd = STDOUT_FILENO;
    }

    if (cell == NULL)
	cell = cell_getthiscell ();

    error = get_vlentry (cell, NULL, volume, auth, &the_vlentry);
    if (error)
	goto out;
    
    if (host == NULL) {
	volser_addr = htonl(the_vlentry.serverNumber[0]);
    } else {
	error = arlalib_name_to_host(host, &volser_addr);
	if (error) {
	    fprintf (stderr, "dump_volume: unknown server %s\n", host);
	    goto out; /* XXX set error? */
	}
    }

    conn_volser = arlalib_getconnbyaddr(cell,
					volser_addr,
					NULL,
					afsvolport,
					VOLSERVICE_ID,
					auth);
    if (conn_volser == NULL) {
	fprintf (stderr, "dump_volume: getconnbyaddr failed\n");
	goto out;
    }

    if (isdigit((unsigned char)volume[0])) {
	char *end;
	long v;
	v = strtoul(volume, &end, 10);
	if (v != ULONG_MAX && *end == '\0')
	    have_volid = 1;
	vol_id = v;
    }
    if (!have_volid) {
	char *v = strdup(volume);
	int type;

	if (v == NULL) {
	    fprintf(stderr, "dump_volume: out of memory\n");
	    goto out;
	}
	type = volname_canonicalize(v);
	free(v);
	vol_id = the_vlentry.volumeId[type];
    }

    if (part == NULL) {
	part_id = the_vlentry.serverPartition[0];
    } else {
	part_id = partition_name2num(part);
	if (part_id == -1) {
	    fprintf (stderr, "dump_volume: unknown partition %s\n", part);
	    error = -1;  /* XXX */
	    goto out;
	}
    }
    
    if (verbose) {
	struct in_addr addr;
	addr.s_addr = volser_addr;
	fprintf (stderr, "dump_volume: server addr is %s\n", inet_ntoa(addr));
	fprintf (stderr, "dump_volume: partition is %d\n", part_id);
	fprintf (stderr, "dump_volume: volume id is %d\n", vol_id);
	fprintf (stderr, "dump_volume: fromtime is %d\n", fromtime);
    }

    error = VOLSER_AFSVolTransCreate(conn_volser,
				     vol_id,
				     part_id,
				     ITReadOnly,
				     &trans_id);
    if (error) {
	fprintf (stderr, "dump_volume: VolTransCreate failed: %s\n",
		 koerr_gettext(error));
	goto out;
    }

    call = rx_NewCall (conn_volser);
    if (call == NULL) {
	fprintf (stderr, "dump_volume: rx_NewCall failed: %s\n",
		 koerr_gettext(error));
	goto out;
    }

    error = StartVOLSER_AFSVolDump(call, trans_id, fromtime);
    if (error) {
	rx_EndCall (call, 0);
	fprintf (stderr, "dump_volume: start AFSVolDump failed: %s\n",
		 koerr_gettext(error));
	goto out;
    }

    ret = rx_Read (call, &size, sizeof(size));

    if (ret != sizeof(size)) {
	ret = conv_to_arla_errno(rx_GetCallError(call));
	rx_EndCall (call, 0);
	fprintf (stderr, "dump_volume: start AFSVolDump failed: %s\n",
		 koerr_gettext(ret));
	goto out;
    }
    size = ntohl(size);

    while (size && (nread = rx_Read (call, buf, min(sizeof(buf), size))) > 0) {
	if (write (fd, buf, nread) != nread) {
	    warn ("write");
	    rx_EndCall(call, 0);
	    goto trans_out;
	}
	size -= nread;
    }

    error = EndVOLSER_AFSVolDump (call);
    if (error) {
	rx_EndCall (call, 0);
	fprintf (stderr, "dump_volume: end AFSVolDump failed: %s\n",
		 koerr_gettext(error));
	goto out;
    }
    rx_EndCall (call, 0);

trans_out:
    ret = 0;

    error = VOLSER_AFSVolEndTrans(conn_volser, trans_id, &ret);
    if (error)
	fprintf (stderr, "dump_volume: VolEndTrans failed: %s\n",
		 koerr_gettext(error));

out:
    if (conn_volser != NULL)
	arlalib_destroyconn (conn_volser);

    if (file != NULL)
	close (fd);
}

static char *vol;
static int32_t fromtime = 0;
static char *server;
static char *part;
static char *cell;
static char *file;
static int noauth;
static int localauth;
static int verbose;
static int helpflag;

static struct agetargs args[] = {
    {"id",	0, aarg_string,  &vol,  "id of volume", "volume",
     aarg_mandatory},
    {"time",	0, aarg_integer,  &fromtime, "dump from time (secs)", NULL},
    {"server",	0, aarg_string,  &server, "what server to use", NULL},
    {"partition",0, aarg_string, &part, "what partition to use", NULL},
    {"cell",	0, aarg_string,  &cell, "what cell to use", NULL},
    {"file",	0, aarg_string,	&file, "file to dump to", NULL},
    {"noauth",	0, aarg_flag,    &noauth, "do not authenticate", NULL},
    {"localauth",0,aarg_flag,    &localauth, "localauth", NULL},
    {"verbose", 0, aarg_flag,	&verbose, "be verbose", NULL},
    {"help",	0, aarg_flag,    &helpflag, NULL, NULL},
    {NULL,      0, aarg_end,	NULL}
};

static void
usage(void)
{
    aarg_printusage(args, "vos dump", "", AARG_AFSSTYLE);
}

int
vos_dump(int argc, char **argv)
{
    int optind = 0;

    noauth = localauth = verbose = 0;
    file = cell = server = part = vol = NULL;

    if (agetarg (args, argc, argv, &optind, AARG_AFSSTYLE)) {
	usage ();
	return 0;
    }

    if (helpflag) {
	usage ();
	return 0;
    }

    argc -= optind;
    argv += optind;

    dump_volume (vol, cell, server, part, file, fromtime,
		 arlalib_getauthflag (noauth, localauth, 0, 0),
		 verbose);
    return 0;
}
