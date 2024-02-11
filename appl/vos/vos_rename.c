/*
 * Copyright (c) 2004 Kungliga Tekniska Högskolan
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

RCSID("$Id: vos_rename.c,v 1.3 2004/07/15 15:19:47 ian Exp $");

static int helpflag;
static char *oldname;
static char *newname;
static char *cell;
static int noauth;
static int localauth;
static int verbose;

static struct agetargs args[] = {
    {"oldname",	0, aarg_string,  &oldname, "old volume name", NULL, 
     aarg_mandatory},
    {"newname", 0, aarg_string, &newname, "what partition to use", NULL, 
     aarg_mandatory},
    {"cell",	0, aarg_string,  &cell, "what cell to use", NULL},
    {"noauth",	0, aarg_flag,    &noauth, "do not authenticate", NULL},
    {"localauth",0,aarg_flag,    &localauth, "localauth", NULL},
    {"verbose", 0, aarg_flag,	&verbose, "be verbose", NULL},
    {"help",	0, aarg_flag,    &helpflag, NULL, NULL},
    {NULL,      0, aarg_end,	NULL}
};

static int
rw_match(struct rx_connection *connvldb, nvldbentry *entry,
	 int32_t server, int32_t part)
{
    int e, same=0;
    for(e=0; e < entry->nServers; e++){
	if(entry->serverFlags[e] & VLSF_RWVOL){
	    if(same_addr_p(connvldb, entry->serverNumber[e], server, &same)){
		return -1;
	    }
	    if((entry->serverPartition[e] == part) || same){
		break;
	    }
	    return -1;
	}
    }
    return e; 
}

static int 
do_rename_volume(struct rx_connection *connvldb, struct rx_connection *connvolser,
		 struct nvldbentry *entry, arlalib_authflags_t auth,
		 char *oldname, char *newname)
{
    int error = 0, index=0, rcode=0;
    int32_t transid;

    strncpy(entry->name, newname, VOLSER_OLDMAXVOLNAME);
  
    error = VL_ReplaceEntryN(connvldb,
			     entry->volumeId[RWVOL],
			     RWVOL,
			     entry,
			     0);
    if(error){
	fprintf(stderr, "Could not update VLDB entry for %u: %s\n",
		entry->volumeId[RWVOL],
		koerr_gettext(error));
	return error;
    }
    printf("Recorded the new name %s in VLDB\n", newname);
    if(entry->flags & VLF_RWEXISTS){
	index = rw_match(connvldb, entry, 0, 0);

	if(index == -1){
	    fprintf(stderr, 
		    "There is a serious discrepancy in VLDB entry for volume %u\n",
		    entry->volumeId[RWVOL]);
	    fprintf(stderr, "try building VLDB from scratch\n");
	    error = VOLSERVLDB_ERROR;
	    return error;
	}
    
	connvolser = arlalib_getconnbyaddr(cell,
					   htonl(entry->serverNumber[index]),
					   NULL,
					   afsvolport,
					   VOLSERVICE_ID,
					   auth);
    
	error = VOLSER_AFSVolTransCreate(connvolser,
					 entry->volumeId[RWVOL],
					 entry->serverPartition[index],
					 ITOffline,
					 &transid);
	if(error){ /* volume does not exist */
	    fprintf(stderr, 
		    "Could not start transaction on the RW volume %u: %s\n",
		    entry->volumeId[RWVOL], 
		    koerr_gettext(error));
	    return error;
	}

	error = VOLSER_AFSVolSetIdsTypes(connvolser, transid, newname,
					 RWVOL,
					 entry->volumeId[RWVOL],
					 entry->volumeId[ROVOL],
					 entry->volumeId[BACKVOL]);
	if(!error){
	    printf("Renamed RW volume %s to %s\n", oldname, newname);
	    error = VOLSER_AFSVolEndTrans(connvolser, transid, &rcode);
	    if(error){
		fprintf(stderr, "Could not end transaction on volume %u: %s\n",
			entry->volumeId[RWVOL], 
			koerr_gettext(error));
		return error;
	    }
	    if(rcode){
		fprintf(stderr, "Could not end transaction on volume %u: %s\n",
			entry->volumeId[RWVOL], 
			koerr_gettext(rcode));
		return rcode;
	    }
      
	} else {
	    fprintf(stderr, "Could not set parameters on volume %u: %s\n",
		    entry->volumeId[RWVOL],
		    koerr_gettext(error));
	    return error;
	}
    }
    return error;
}

static 
void usage(void)
{
    aarg_printusage(args, "vos rename", "", AARG_AFSSTYLE);
}

int 
vos_rename(int argc, char **argv)
{
    const char *host;
    int optind = 0, error = 0, islocked = 0, vcode = 0;
    struct nvldbentry entry;
    struct rx_connection *connvldb = NULL, *connvolser = NULL;
    arlalib_authflags_t auth;

    helpflag = noauth = localauth = verbose = 0;
    host = oldname = newname = cell = NULL;

    if(agetarg(args, argc, argv, &optind, AARG_AFSSTYLE)){
	usage();
	return 0;
    }
    if(helpflag){
	usage();
	return 0;
    }

    argc -= optind;
    argv += optind;

    if(oldname == NULL || oldname[0] == '\0'){
	usage();
	return 0;
    }
    if(newname == NULL || newname[0] == '\0'){
	usage();
	return 0;
    }
  
    find_db_cell_and_host((const char **)&cell, &host);
    if(cell == NULL){
	fprintf (stderr, "Unable to find cell of host '%s'\n", host);
	return 1;
    }
    if(host == NULL){
	fprintf (stderr, "Unable to find DB server in cell '%s'\n", cell);
	return 1;
    }
    auth = arlalib_getauthflag(noauth,localauth,0,0);
    connvldb = arlalib_getconnbyname(cell, 
				     host,
				     afsvldbport, 
				     VLDB_SERVICE_ID,
				     auth);
    if(connvldb == NULL){
	fprintf(stderr, "vos_rename: arlalib_getconnbyaddr: %s\n",
		koerr_gettext(error));
	return 1;
    }
    error = get_vlentry(cell, NULL, oldname, auth, &entry);
    if(error){
	fprintf(stderr, "vos_rename: get_vlentry: %s\n",
		koerr_gettext(error));
	return error;
    }
    error = VL_SetLock(connvldb, entry.volumeId[RWVOL], RWVOL,
		       VLOP_DELETE);
    if(error){
	fprintf(stderr, "vos_rename: VL_SetLock: %s\n",
		koerr_gettext(error));
    }
    islocked = 1;
    error = do_rename_volume(connvldb, connvolser, &entry, auth, 
			     oldname, newname);

    if(islocked){
	vcode = VL_ReleaseLock(connvldb, entry.volumeId[RWVOL], RWVOL,
			       LOCKREL_TIMESTAMP | LOCKREL_OPCODE | LOCKREL_AFSID);
	if(vcode){
	    fprintf(stderr, "Couldn't release lock on volume entry for %u: %s\n",
		    entry.volumeId[RWVOL],
		    koerr_gettext(vcode));
	}
    }
    if(connvldb != NULL)
	arlalib_destroyconn(connvldb);
    if(connvolser != NULL)
	arlalib_destroyconn(connvolser);
    return error;
}
