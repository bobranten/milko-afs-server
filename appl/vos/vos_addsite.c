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

RCSID("$Id: vos_addsite.c,v 1.6 2004/07/15 23:23:11 ian Exp $");

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


int
same_addr_p(struct rx_connection *connvldb, int32_t server1, 
	    int32_t server2, int *result)
{

    bulkaddrs addrs;
    afsUUID uuid;
    int unique =0;
    int32_t nentries, *addrp;
    int error,i;
    struct ListAddrByAttributes attrs;
  
    memset(&attrs, 0, sizeof(attrs));
    memset(&addrs, 0, sizeof(addrs));
    memset(&uuid, 0, sizeof(uuid));
  
    attrs.Mask = VLADDR_IPADDR;
    attrs.ipaddr = server1;
  
    error = VL_GetAddrsU(connvldb, &attrs, &uuid, 
			 &unique, &nentries,&addrs);
    if(error){
	fprintf(stderr, "same_addr_p: %s\n",
		koerr_gettext(error));
	return -1;
    }
  
    addrp = addrs.val;
    for(i=0; i<nentries;i++, addrp++){
	if( server2 == *addrp){
	    *result = 1;
	    break;
	}
    }
    return 0;
}

static int
vos_add_site(char *servername, char *partition, char *volname, 
	     arlalib_authflags_t auth)
{
    struct rx_connection *connvldb = NULL;
    const char *host = NULL;
    int error=0, same=0, j, num_ro=0, islocked, vcode;
    nvldbentry the_vlentry;
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
	fprintf(stderr, "vos_add_site: arlalib_getconnbyname: %s\n",
		koerr_gettext(error));

	goto out;
    }
    
  
    error = get_vlentry(cell, NULL, volname, auth, &the_vlentry);
    if(error){
	fprintf(stderr, "vos_add_site: get_vlentry: %s\n",
		koerr_gettext(error));
	goto out;
    }
    /* OpenAFS says VLOP_ADDSITE, but volser/volser.h says that's bogus
       Anyway, they map to the same value... */
    error = VL_SetLock(connvldb, the_vlentry.volumeId[RWVOL], RWVOL,
		       VLOP_DELETE);
    if(error){
	fprintf(stderr, "vos_add_site: VL_SetLock: %s\n",
		koerr_gettext(error));
	goto out;
    }

    islocked = 1;
    if(the_vlentry.nServers >= NMAXNSERVERS){
	fprintf(stderr, "Total number of entries will exceed %u\n",
		NMAXNSERVERS);
	goto out;

    }
  
  
    error = arlalib_name_to_host(servername, &server);
    server = ntohl(server);
    if(error){
	fprintf(stderr, "vos_add_site: arlalib_name_to_host: %s\n",
		koerr_gettext(error));
	goto out;
    }   
  

    for(j=0; j< the_vlentry.nServers; j++){
	if(the_vlentry.serverFlags[j] & VLSF_ROVOL){
	    num_ro++;
	    if(same_addr_p(connvldb,  server , the_vlentry.serverNumber[j], &same)){
		fprintf(stderr, "vos_add_site: same_addr_p failed\n");
		goto out;
	    } 
	
	    if(same) {
		char part_name[17];
		partition_num2name(the_vlentry.serverPartition[j], 
				   part_name, sizeof(part_name));
		fprintf(stderr, 
			"RO already exists on %s %s\nMultiple ROs on a single server aren't allowed", 
			servername, part_name);
		goto out;
	    }
	}
    
    }

    if(num_ro >= NMAXNSERVERS -1){
	fprintf(stderr, "Total number of sites will exceed %u\n",
		NMAXNSERVERS-1);
	goto out;
    }

    part = partition_name2num( partition );
  
    if( part == -1 ){
	fprintf(stderr, "Partition %s doesn't exist on server %s",
		partition, servername );
	goto out;
    }
    
    printf("Adding a new site ...");

    the_vlentry.serverNumber[the_vlentry.nServers] = server;
    the_vlentry.serverPartition[the_vlentry.nServers] = part;
    the_vlentry.serverFlags[the_vlentry.nServers] = (VLSF_ROVOL | VLSF_DONTUSE);
    the_vlentry.nServers++;
 
    error = VL_ReplaceEntryN(connvldb,
			     the_vlentry.volumeId[RWVOL],
			     RWVOL,
			     &the_vlentry,
			     LOCKREL_OPCODE | LOCKREL_AFSID | LOCKREL_TIMESTAMP);
    if (error) {
	fprintf (stderr, "vos_add_site: VL_ReplaceEntryN: %s\n", 
		 koerr_gettext (error));
	goto out;
    }
    islocked = 0;

 out:
    if(islocked){
	vcode = VL_ReleaseLock(connvldb, the_vlentry.volumeId[RWVOL], RWVOL,
			       LOCKREL_TIMESTAMP | LOCKREL_OPCODE | LOCKREL_AFSID);
	if(vcode){
	    fprintf(stderr, "Couldn't release lock on volume entry for %u: %s\n",
		    the_vlentry.volumeId[RWVOL],
		    koerr_gettext(vcode));
	}
    }

    if(connvldb != NULL)
	arlalib_destroyconn(connvldb);

    printf(" done\n");
    return error;
}

static void usage(void)
{
    aarg_printusage(args, "vos addsite", "", AARG_AFSSTYLE);
}

int 
vos_addsite(int argc, char **argv)
{
    int optind = 0, error=0;
    helpflag = noauth = localauth = verbose = 0;
    server = cell = vol = partition = NULL;

  
    if( agetarg(args, argc, argv, &optind, AARG_AFSSTYLE) ){
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

  
    error = vos_add_site(server, partition, vol, 
			 arlalib_getauthflag(noauth,localauth,0,0));
    if(error == -1){
	fprintf(stderr, "vos_addsite: vos_add_site failed\n");
    }
  

    return 0;
}
