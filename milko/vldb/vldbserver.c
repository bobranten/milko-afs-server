/*
 * Copyright (c) 1995 - 2001 Kungliga Tekniska Högskolan
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

#include "vldb_locl.h"

RCSID("$Id: vldbserver.c,v 1.48 2005/03/27 17:03:54 tol Exp $");

/*
 * The rpc - calls
 */

int
SVL_CreateEntry(struct rx_call *call, 
		const vldbentry *newentry) 
{
    char *name;
    disk_vlentry diskentry;
    disk_vlentry tempentry;

    mlog_log (MDEBVL, "SVL_CreateEntry (name=%s, ids=%d,%d,%d flags=%d)\n",
	      newentry->name,
	      newentry->volumeId[RWVOL],
	      newentry->volumeId[ROVOL],
	      newentry->volumeId[BACKVOL],
	      newentry->flags);


    if (!sec_is_superuser(call))
	return VL_PERM;

    if ((vldb_id_to_name(newentry->volumeId[RWVOL], &name) == 0) ||
	(vldb_id_to_name(newentry->volumeId[ROVOL], &name) == 0) ||
	(vldb_id_to_name(newentry->volumeId[BACKVOL], &name) == 0)) {
	free(name);
	mlog_log (MDEBVL, "SVL_CreateEntry: id exists\n");
	return VL_NAMEEXIST;
    }

    if (vldb_read_entry(newentry->name, &tempentry) == 0) {
	mlog_log (MDEBVL, "SVL_CreateEntry: name exists\n");
	return VL_NAMEEXIST;
    }

    vldb_entry_to_disk(newentry, &diskentry);

    if (vldb_write_entry(&diskentry) != 0)
	return VL_IO;

    if (vldb_write_id(newentry->name,
		      newentry->volumeId[RWVOL]) != 0)
	return VL_IO; /* XXX rollback */

    if (vldb_write_id(newentry->name,
		      newentry->volumeId[ROVOL]) != 0)
	return VL_IO; /* XXX rollback */

    if (vldb_write_id(newentry->name,
		      newentry->volumeId[BACKVOL]) != 0)
	return VL_IO; /* XXX rollback */

    vldb_flush();

    vldb_free_diskentry(&diskentry);

    return 0;
}

int 
SVL_DeleteEntry(struct rx_call *call, 
		const int32_t Volid, 
		const int32_t voltype)
{
    disk_vlentry entry;
    char *name;
    int ret;

    mlog_log (MDEBVL, "SVL_DeleteEntry (Volid=%d,Voltype=%d)\n", 
		  Volid, voltype);

    if (!sec_is_superuser(call)) {
	ret =  VL_PERM;
	goto out;
    }
    
    if (voltype != RWVOL &&
	voltype != ROVOL &&
	voltype != BACKVOL) {
	ret =  VL_BADVOLTYPE;
	goto out;
    }
    
    if (vldb_id_to_name(Volid, &name)) {
	ret =  VL_NOENT;
	goto out;
    }
    
    if (vldb_read_entry(name, &entry) != 0) {
	ret =  VL_NOENT;
	goto out;
    }
    
    if (entry.volumeId[voltype] != Volid) {
	ret =  VL_NOENT;
	goto out;
    }
    
    if (vldb_delete_id(name, entry.volumeId[RWVOL])) {
	mlog_log (MDEBVL, "SVL_DeleteEntry failed to remove RW id %d\n",
		  entry.volumeId[RWVOL]);
	ret =  VL_IO;
	goto out;
    }
    if (vldb_delete_id(name, entry.volumeId[ROVOL])) {
	mlog_log (MDEBVL, "SVL_DeleteEntry failed to remove RO id %d\n",
		  entry.volumeId[ROVOL]);
	ret =  VL_IO;
	goto out;
    }
    if (vldb_delete_id(name, entry.volumeId[BACKVOL])) {
	mlog_log (MDEBVL, "SVL_DeleteEntry failed to remove BK id %d\n",
		  entry.volumeId[BACKVOL]);
	ret =  VL_IO;
	goto out;
    }
    if (vldb_delete_entry(name)) {
	mlog_log (MDEBVL, "SVL_DeleteEntry failed to remove data\n");
	ret =  VL_IO;
	goto out;
    }
    
    free(name);

    vldb_flush();

    ret = 0;
    
 out:
    mlog_log (MDEBVL, "SVL_DeleteEntry returns %d\n", ret);

    return ret;
}

/*
 *
 */

int
SVL_GetEntryByID(struct rx_call *call,
		 const int32_t Volid, 
		 const int32_t voltype, 
		 vldbentry *entry) 
{
    disk_vlentry diskentry;
    char *name;
    mlog_log (MDEBVL, "SVL_GetEntryByID (Volid=%d,Voltype=%d)\n", 
		  Volid, voltype);

    if (vldb_id_to_name(Volid, &name))
	return VL_NOENT;

    if (vldb_read_entry(name, &diskentry) != 0)
	return VL_NOENT;

    vldb_disk_to_entry(&diskentry, entry);

    free(name);

    return 0;
}

/*
 *
 */

int
SVL_GetEntryByName(struct rx_call *call, 
		   const char *volumename, 
		   vldbentry *entry) 
{
    disk_vlentry diskentry;

    mlog_log (MDEBVL, "SVL_GetEntryByName (volumename = %s)\n", 
		  volumename);

    if (isdigit(volumename[0])) {
	return SVL_GetEntryByID(call, atol(volumename), 0 /* XXX */, entry);
    }

    if (vldb_read_entry(volumename, &diskentry) != 0)
	return VL_NOENT;

    vldb_disk_to_entry(&diskentry, entry);
    
    return 0;
}

/*
 *
 */

int 
SVL_GetNewVolumeId (struct rx_call *call, 
		    const int32_t bumpcount,
		    int32_t *newvolumid)
{
    mlog_log (MDEBVL, "SVL_GetNewVolumeId(bumpcount=%d)\n", bumpcount) ;
    
    if (!sec_is_superuser(call))
	return VL_PERM;

    *newvolumid = vl_header.MaxVolumeId;
    mlog_log (MDEBVL, "   returning low volume id = %d\n", *newvolumid);
    vl_header.MaxVolumeId += bumpcount;
    vldb_write_header();

    vldb_flush();

    return 0;
}

/*
 *
 */

int
SVL_ReplaceEntry (struct rx_call *call, 
		  const int32_t Volid, 
		  const int32_t voltype,
		  const vldbentry *newentry,
		  const int32_t ReleaseType) 
{
    mlog_log (MDEBVL, "SVL_ReplaceEntry\n") ;

    if (!sec_is_superuser(call))
	return VL_PERM;

    return VL_PERM ;
}

/*
 *
 */

int
SVL_UpdateEntry (struct rx_call *call, 
		 const int32_t Volid, 
		 const int32_t voltype, 
		 const VldbUpdateEntry *UpdateEntry,
		 const int32_t ReleaseType)
{
    mlog_log (MDEBVL, "SVL_UpdateEntry\n") ;

    if (!sec_is_superuser(call))
	return VL_PERM;

    return VL_PERM ;
}

/*
 *
 */

int 
SVL_SetLock (struct rx_call *call, 
	     const int32_t Volid, 
	     const int32_t voltype,
	     const int32_t voloper) 
{
    mlog_log (MDEBVL, "SVL_SetLock\n") ;

    if (!sec_is_superuser(call))
	return VL_PERM;

    return 0;
}

/*
 *
 */

int
SVL_ReleaseLock (struct rx_call *call, 
		 const int32_t volid,
		 const int32_t voltype, 
		 const int32_t ReleaseType) 
{
    mlog_log (MDEBVL, "SVL_ReleaseLock\n") ;

    if (!sec_is_superuser(call))
	return VL_PERM;


    return 0;
}

/*
 *
 */

int
SVL_ListEntry (struct rx_call *call, 
	       const int32_t previous_index, 
	       int32_t *count,
	       int32_t *next_index, 
	       vldbentry *entry) 
{
    mlog_log (MDEBVL, "SVL_ListEntry\n") ;
    return VL_PERM ;
}

/*
 *
 */

int
SVL_ListAttributes (struct rx_call *call, 
		    const VldbListByAttributes *attributes,
		    int32_t *nentries,
		    bulkentries *blkentries) 
{
    mlog_log (MDEBVL, "SVL_ListAttributes\n") ;
    return VL_PERM ;
}

/*
 *
 */

int
SVL_GetStats (struct rx_call *call, 
	      vldstats *stats,
	      vital_vlheader *vital_header) 
{
    mlog_log (MDEBVL, "SVL_GetStats") ;
    return VL_PERM ;
}

/*
 *
 */

int 
SVL_Probe(struct rx_call *call)
{
    mlog_log (MDEBVL, "SVL_Probe\n") ;
    return 0;
}

/*
 *
 */

int
SVL_CreateEntryN(struct rx_call *call,
		 const nvldbentry *entry)
{
    char *name;
    disk_vlentry diskentry;
    disk_vlentry tempentry;

    mlog_log (MDEBVL, "SVL_CreateEntryN (name=%s, ids=%d,%d,%d flags=%d)\n",
	      entry->name,
	      entry->volumeId[RWVOL],
	      entry->volumeId[ROVOL],
	      entry->volumeId[BACKVOL],
	      entry->flags);

    if (!sec_is_superuser(call))
	return VL_PERM;


    if ((vldb_id_to_name(entry->volumeId[RWVOL], &name) == 0) ||
	(vldb_id_to_name(entry->volumeId[ROVOL], &name) == 0) ||
	(vldb_id_to_name(entry->volumeId[BACKVOL], &name) == 0)) {
	free(name);
	mlog_log (MDEBVL, "SVL_CreateEntryN: id exists\n");
	return VL_NAMEEXIST;
    }

    if (vldb_read_entry(entry->name, &tempentry) == 0) {
	mlog_log (MDEBVL, "SVL_CreateEntryN: name exists\n");
	return VL_NAMEEXIST;
    }

    vldb_nentry_to_disk(entry, &diskentry);

    if (vldb_write_entry(&diskentry) != 0)
	return VL_IO;

    if (vldb_write_id(entry->name,
		      entry->volumeId[RWVOL]) != 0)
	return VL_IO; /* XXX rollback */

    if (vldb_write_id(entry->name,
		      entry->volumeId[ROVOL]) != 0)
	return VL_IO; /* XXX rollback */

    if (vldb_write_id(entry->name,
		      entry->volumeId[BACKVOL]) != 0)
	return VL_IO; /* XXX rollback */

    vldb_free_diskentry(&diskentry);

    vldb_flush();

    return 0;
}

/*
 *
 */

int
SVL_GetEntryByIDN(struct rx_call *call,
		  const int32_t Volid,
		  const int32_t voltype,
		  nvldbentry *entry)
{
    disk_vlentry diskentry;
    char *name;
    mlog_log (MDEBVL, "SVL_GetEntryByIDN (Volid=%d,Voltype=%d)\n", 
	      Volid, voltype);

    if (vldb_id_to_name(Volid, &name))
	return VL_NOENT;

    if (vldb_read_entry(name, &diskentry) != 0)
	return VL_NOENT;

    vldb_disk_to_nentry(&diskentry, entry);

    free(name);

    return 0;
}

/*
 *
 */

int
SVL_GetEntryByNameN(struct rx_call *call, 
		    const char *volumename, 
		    nvldbentry *entry) 
{
    disk_vlentry diskentry;

    mlog_log (MDEBVL, "SVL_GetEntryByNameN (volumename = %s)\n", 
	      volumename);

    if (isdigit(volumename[0])) {
	return SVL_GetEntryByIDN(call, atol(volumename), 0 /* XXX */, entry);
    }

    if (vldb_read_entry(volumename, &diskentry) != 0)
	return VL_NOENT;

    vldb_disk_to_nentry(&diskentry, entry);
    
    return 0;
}

#ifdef notyet
/*
 *
 */

int
SVL_GetEntryByNameU(struct rx_call *call, 
		    const char *volumename, 
		    uvldbentry *entry) 
{
    mlog_log (MDEBVL, "SVL_GetEntryByNameU %s\n", volumename);
    memset(entry, 0, sizeof(*entry));
    
    return RXGEN_OPCODE;
}
#endif

/*
 *
 */

int
SVL_ListAttributesN (struct rx_call *call, 
		     const VldbListByAttributes *attributes,
		     int32_t *nentries,
		     nbulkentries *blkentries) 
{
    mlog_log (MDEBVL, "SVL_ListAttributesN\n");
    mlog_log (MDEBVL, "  attributes: Mask=(%d=", attributes->Mask);

    if (attributes->Mask & VLLIST_SERVER)
	mlog_log (MDEBVL, "SERVER ");
    if (attributes->Mask & VLLIST_PARTITION)
	mlog_log (MDEBVL, "PARTITION ");
    if (attributes->Mask & VLLIST_VOLUMEID)
	mlog_log (MDEBVL, "VOLUMEID ");
    if (attributes->Mask & VLLIST_FLAG)
	mlog_log (MDEBVL, "FLAG");

    mlog_log (MDEBVL, ") server=%d partition=%d volumetype=%d volumeid=%d flag=%d\n",
	   attributes->server,
	   attributes->partition,
	   attributes->volumetype,
	   attributes->volumeid,
	   attributes->flag);

    *nentries = 1;

    blkentries->len = 0;
    blkentries->val = NULL;

    return VL_PERM;
}

#ifdef notyet
/*
 *
 */

int
SVL_ListAttributesU(struct rx_call *call, 
		    const VldbListByAttributes *attributes,
		    int32_t *nentries,
		    ubulkentries *blkentries) 
{
    mlog_log (MDEBVL, "SVL_ListAttributesU\n") ;
    *nentries = 0;
    blkentries->len = 0;
    blkentries->val = NULL;
    return 0;
}
#endif

/*
 *
 */

int
SVL_UpdateEntryByName(struct rx_call *call,
		      const char *volname,
		      const struct VldbUpdateEntry *UpdateEntry,
		      const int32_t ReleaseType)
{
    mlog_log (MDEBVL, "SVL_UpdateEntryByName (not implemented)\n");

    if (!sec_is_superuser(call))
	return VL_PERM;

    return VL_PERM;
}

/*
 *
 */

int
SVL_GetAddrsU(struct rx_call *call,
	      const struct ListAddrByAttributes *inaddr,
	      struct afsUUID *uuid,
	      int32_t *uniq,
	      int32_t *nentries,
	      bulkaddrs *addrs)
{
    mlog_log (MDEBVL, "SVL_GetAddrsU (not implemented)\n");
    return RXGEN_OPCODE;
}

/*
 *
 */

int
SVL_RegisterAddrs(struct rx_call *call,
		  const struct afsUUID *uid,
		  const int32_t spare,
		  const bulkaddrs *addrs)
{
    mlog_log (MDEBVL, "SVL_RegistersAddrs (not implemented)\n");

    if (!sec_is_superuser(call))
	return VL_PERM;

    return 0;
}

#ifdef notyet
/*
 *
 */

int
SVL_CreateEntryU(struct rx_call *call,
		 const struct uvldbentry *newentry)
{
    mlog_log (MDEBVL, "SVL_CreateEntryU (not implemented)\n");

    if (!sec_is_superuser(call))
	return VL_PERM;

    return VL_PERM;
}
#endif

#ifdef notyet
/*
 *
 */

int
SVL_ReplaceEntryU(struct rx_call *call)
{
    mlog_log (MDEBVL, "SVL_ReplaceEntryU (not implemented)\n");

    if (!sec_is_superuser(call))
	return VL_PERM;

    return VL_PERM;
}
#endif

/*
 *
 */

int
SVL_ReplaceEntryN(struct rx_call *call,
		  const int32_t Volid,
		  const int32_t voltype,
		  const struct nvldbentry *newentry,
		  const int32_t ReleaseType)
{
    mlog_log (MDEBVL, "SVL_ReplaceEntryN (not implemented)\n");

    if (!sec_is_superuser(call))
	return VL_PERM;

    return VL_PERM;
}

/*
 *
 */

int
SVL_ChangeAddrs(struct rx_call *call,
		const int32_t old_ip,
		const int32_t new_ip)
{
    mlog_log (MDEBVL, "SVL_ChangeAddrs (not implemented)\n");

    if (!sec_is_superuser(call))
	return VL_PERM;

    return VL_PERM;
}

#ifdef notyet
/*
 *
 */

int
SVL_GetEntryByIDU(struct rx_call *call)
{
    mlog_log (MDEBVL, "SVL_GetEntryByIDU (not implemented)\n");
    return VL_PERM;
}
#endif

/*
 *
 */

int
SVL_ListEntryN(struct rx_call *call,
	       int32_t previous_index,
	       int32_t *count,
	       int32_t *next_index,
	       nvldbentry *entry)
{
    mlog_log (MDEBVL, "SVL_ListEntryN (not implemented)\n");
    return VL_PERM;
}

#ifdef notyet
/*
 *
 */

int
SVL_ListEntryU(struct rx_call *call)
{
    mlog_log (MDEBVL, "SVL_ListEntryU (not implemented)\n");
    return VL_PERM;
}
#endif

/*
 *
 */

int
SVL_GetAddrs(struct rx_call *call,
	     const int32_t handle,
	     const int32_t spare,
	     struct VL_Callback *spare3,
	     int32_t *nentries,
	     bulkaddrs *blkaddr)
{
    mlog_log (MDEBVL, "SVL_GetAddrs (not implemented)\n");
    return VL_PERM;
}

#ifdef notyet
/*
 *
 */

int
SVL_LinkedListN(struct rx_call *call)
{
    mlog_log (MDEBVL, "SVL_LinkedListN (not implemented)\n");
    return VL_PERM;
}

/*
 *
 */

int
SVL_LinkedListU(struct rx_call *call)
{
    mlog_log (MDEBVL, "SVL_LinkedListU (not implemented)\n");
    return VL_PERM;
}
#endif

int
SVL_ListAttributesN2(struct rx_call *call,
		     const struct VldbListByAttributes *attributes,
		     const char *volumename,
		     const int32_t startindex,
		     int32_t *nentries,
		     nbulkentries *blkentries,
		     int32_t *nextstartindex)
{
    mlog_log (MDEBVL, "SVL_ListAttributesN2 (not implemented)\n");
    return VL_PERM;
}
/*
 *
 */

static struct rx_service *vldbservice = NULL;
static struct rx_service *ubikservice = NULL;

static char *cell = NULL;
static char *realm = NULL;
static char *srvtab_file = NULL;
static char *log_file = "syslog";
static char *debug_levels = NULL;
static int no_auth = 0;
static int do_help = 0;
static char *databasedir = NULL;
static int do_create = 0;

static struct agetargs args[] = {
    {"cell",	0, aarg_string,    &cell, "what cell to use"},
    {"realm",	0, aarg_string,	  &realm, "what realm to use"},
    {"debug",  'd', aarg_string,  &debug_levels, "debug level"},
    {"log",	'l',	aarg_string,	&log_file,
     "where to write log (stderr, syslog (default), or path to file)"},
    {"srvtab", 0, aarg_string,    &srvtab_file, "what srvtab to use"},
    {"noauth", 0,  aarg_flag,	  &no_auth, "disable authentication checks"},
    {"help",  'h', aarg_flag,      &do_help, "help"},
    {"dbdir",  0, aarg_string,    &databasedir, "where to store the db"},
    {"create",  0, aarg_flag,      &do_create, "create new database"},
    { NULL, 0, aarg_end, NULL }
};

static void
usage(int exit_code)
{
    aarg_printusage (args, NULL, "", AARG_GNUSTYLE);
    exit (exit_code);
}

int
main(int argc, char **argv) 
{
    Log_method *method;
    int optind = 0;
    int ret;
    
    setprogname (argv[0]);

    if (agetarg (args, argc, argv, &optind, AARG_GNUSTYLE)) {
	usage (1);
    }

    argc -= optind;
    argv += optind;

    if (argc) {
	printf("unknown option %s\n", *argv);
	return 1;
    }

    if (do_help)
	usage(0);

    if (no_auth)
	sec_disable_superuser_check ();

    method = log_open (getprogname(), log_file);
    if (method == NULL)
	errx (1, "log_open failed");
    cell_init(0, method);
    ports_init();

    mlog_loginit (method, milko_deb_units, MDEFAULT_LOG);

    if (debug_levels)
	mlog_log_set_level (debug_levels);

    if (cell)
	cell_setthiscell (cell);

    network_kerberos_init (srvtab_file);
    
    if (do_create) {
	vldb_create (databasedir);
	vldb_close();
	return 0;
    }

    vldb_init(databasedir);

    ret = network_init(htons(afsvldbport), "vl", VLDB_SERVICE_ID, 
		       VL_ExecuteRequest, &vldbservice, realm);
    if (ret)
	errx (1, "network_init failed with %d", ret);
    
    ret = network_init(htons(afsvldbport), "ubik", VOTE_SERVICE_ID, 
		       Ubik_ExecuteRequest, &ubikservice, realm);
    if (ret)
	errx (1, "network_init failed with %d", ret);

    printf("Milko vldbserver %s-%s started\n", PACKAGE, VERSION);

    rx_SetMaxProcs(vldbservice,5) ;
    rx_SetMaxProcs(ubikservice,5) ;
    rx_StartServer(1) ;

    abort() ; /* should not get here */
    return 0;
}
