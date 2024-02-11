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

RCSID("$Id: vos_remsite.c,v 1.2 2004/07/15 23:22:15 ian Exp $");

static int helpflag;
static char *server;
static char *cell;
static char *vol;
static char *partition;
static int noauth;
static int localauth;
static int verbose;

static struct agetargs args[] = {
    {"server",	0, aarg_string,  &server, "what server to use", NULL, 
     aarg_mandatory},
    {"partition", 0, aarg_string, &partition, "what partition to use", NULL, 
     aarg_mandatory},
    {"id",	0, aarg_string,  &vol,  "id of volume", "volume",
     aarg_mandatory},
    {"cell",	0, aarg_string,  &cell, "what cell to use", NULL},
    {"noauth",	0, aarg_flag,    &noauth, "do not authenticate", NULL},
    {"localauth",0,aarg_flag,    &localauth, "localauth", NULL},
    {"verbose", 0, aarg_flag,	&verbose, "be verbose", NULL},
    {"help",	0, aarg_flag,    &helpflag, NULL, NULL},
    {NULL,      0, aarg_end,	NULL}
};


static int ro_match(struct rx_connection *connvldb, nvldbentry *entry, 
		    int32_t server, int32_t part)
{
    int e, same=0;
    
    for(e=0; e < entry->nServers; e++){
	if(entry->serverFlags[e] & VLSF_ROVOL){
	    if(same_addr_p(connvldb, entry->serverNumber[e], server, &same)){
		return -1;
	    }
	    if((entry->serverPartition[e] == part) && same){
		break;
	    }
	}
    }
    if( e>= entry->nServers){
	/* Nope. Didn't find it */
	return -1; 
    }
    return e; 
}
static int vos_remove_site(char *servername, char *partition, char *volname,
			   arlalib_authflags_t auth)
{
    struct rx_connection *connvldb = NULL;
    nvldbentry entry;
    const char *host = NULL;
    int error=0, islocked, vcode, index;
    uint32_t server, part;
    
    find_db_cell_and_host( (const char **)&cell, &host);
    if (cell == NULL) {
	fprintf (stderr, "Unable to find cell of host '%s'\n", host);
	goto out;
    }
    
    if (host == NULL) {
	fprintf (stderr, "Unable to find DB server in cell '%s'\n", cell);
	goto out;
    }
    
    connvldb = arlalib_getconnbyname(cell, 
				     host,
				     afsvldbport, 
				     VLDB_SERVICE_ID,
				     auth);
    if(connvldb == NULL){
	fprintf(stderr, "vos_add_site: arlalib_getconnbyaddr: %s\n",
		koerr_gettext(error));
	
	goto out;
    }
    
    error = get_vlentry(cell, NULL, volname, auth, &entry);
    if(error){
	fprintf(stderr, "vos_add_site: get_vlentry: %s\n",
		koerr_gettext(error));
	goto out;
    }
    /* OpenAFS says VLOP_ADDSITE, but volser/volser.h says that's bogus
       Anyway, they map to the same value... */
    error = VL_SetLock(connvldb, entry.volumeId[RWVOL], RWVOL,
		       VLOP_DELETE);
    
    if(error){
	fprintf(stderr, "vos_add_site: VL_SetLock: %s\n",
		koerr_gettext(error));
	goto out;
    }
    islocked = 1;
    
    error = arlalib_name_to_host(servername, &server);
    server = ntohl(server);
    if(error){
	fprintf(stderr, "vos_add_site: arlalib_name_to_host: %s\n",
		koerr_gettext(error));
	goto out;
    }
    part = partition_name2num( partition );
    
    if( part == -1 ){
	fprintf(stderr, "Partition %s doesn't exist on server %s",
		partition, servername );
	goto out;
    }
    
    index = ro_match(connvldb, &entry, server, part);
    if(index == -1){
	fprintf(stderr, "This site is not a replication site\n");
	goto out;
    }
    
    
    entry.serverNumber[index] = 0;
    entry.serverPartition[index] = 0;
    for(index++; index < entry.nServers; index++){
	entry.serverNumber[index-1] = entry.serverNumber[index];
	entry.serverPartition[index-1] = entry.serverPartition[index];
	entry.serverFlags[index-1] = entry.serverFlags[index];
    }
    entry.nServers--;
    
    if((entry.nServers == 1) && (entry.flags & VLF_RWEXISTS))
	entry.flags &= ~ VLF_ROEXISTS;
    if(entry.nServers < 1){
	/* last reference to volume. Does this happen? */
	printf("Deleing the VLDB entry for %u ...", entry.volumeId[RWVOL]);
	error = VL_DeleteEntry(connvldb, entry.volumeId[RWVOL], ROVOL);
	if( error ){
	    fprintf(stderr, "Could not delete VLDB entry for volume %u: %s\n",
		    entry.volumeId[RWVOL],
		    koerr_gettext(error));
	    goto out;
	}
	printf(" done\n");
    }
    printf("Deleting the replication site for volume %u...",
	   entry.volumeId[RWVOL]);
    error = VL_ReplaceEntryN(connvldb, 
			     entry.volumeId[RWVOL],
			     RWVOL,
			     &entry,
			     LOCKREL_OPCODE | LOCKREL_AFSID | LOCKREL_TIMESTAMP);
    if( error ){
	fprintf(stderr, "Couldn't delete replication site for volume %u: %s\n",
		entry.volumeId[RWVOL],
		koerr_gettext(error));
	goto out;
    }
    printf(" done \n");
    islocked = 0;
 out:
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
    
    return error;
}

static void usage(void)
{
    aarg_printusage(args, "vos remsite", "", AARG_AFSSTYLE);
}

int vos_remsite(int argc, char **argv)
{
    int optind = 0, error = 0;
    helpflag = noauth = localauth = verbose = 0;
    server = cell = vol = partition = NULL;
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
    
    
    if( vol == NULL || vol[0] == '\0'){
	usage();
	return 0;
    }
    if(server == NULL || server[0] == '\0'){
	usage();
	return 0;
    }
    if(partition == NULL || partition[0] == '\0'){
	usage();
	return 0;
    }
    error = vos_remove_site( server,  partition, vol, 
			     arlalib_getauthflag(noauth,localauth,0,0));
    if( error )
	return 1;
    return 0;
}

