/*
 * Copyright (c) 2001 Kungliga Tekniska Högskolan
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

#include "mdb_locl.h"

RCSID("$Id: ndbm.c,v 1.6 2003/02/15 23:51:53 lha Exp $");
#define NDBM 1
#if defined(NDBM) || defined(HAVE_NEW_DB)

#if defined(HAVE_GDBM_NDBM_H)
#include <gdbm/ndbm.h>
#elif defined(HAVE_DBM_H)
#include <dbm.h>
#elif defined(HAVE_NDBM_H)
#include <ndbm.h>
#endif
#include <gdbm.h>
struct ndbm_db {
    GDBM_FILE db;
    int lock_fd;
};

static int
NDBM_close(MDB *db)
{
    struct ndbm_db *d = db->db;
    gdbm_close(d->db);
    free(d);
    return 0;
}

static int
NDBM_open(MDB *db)
{
    struct ndbm_db *d = malloc(sizeof(*d));

    assert(db);

    d->db = gdbm_open((char *)db->name, 512, db->flags, db->mode, NULL);
    if(d->db == NULL)
	return errno;

    db->db = d;
    assert(db->db);

    return 0;
}

static int
NDBM_store(MDB *db, struct mdb_datum *key, struct mdb_datum *value)
{
    struct ndbm_db *d = (struct ndbm_db *)db->db;
    datum k, v;

    k.dptr = key->data;
    k.dsize = key->length;
    v.dptr = value->data;
    v.dsize = value->length;

    return gdbm_store(d->db, k, v, GDBM_REPLACE);
}

static int
NDBM_fetch(MDB *db, struct mdb_datum *key, struct mdb_datum *value)
{
    struct ndbm_db *d = (struct ndbm_db *)db->db;
    datum k;
    datum v;

    k.dptr = key->data;
    k.dsize = key->length;

    v = gdbm_fetch(d->db, k);

    if (v.dptr == NULL)
	return ENOENT;

    value->data = v.dptr;
    value->length = v.dsize;

    return 0;
}

static int
NDBM_delete(MDB *db, struct mdb_datum *key)
{
    struct ndbm_db *d = (struct ndbm_db *)db->db;
    datum k;

    k.dptr = key->data;
    k.dsize = key->length;

    return gdbm_delete(d->db, k);
}

static int
NDBM_flush(MDB *db)
{
    int ret;

    ret = NDBM_close(db);
    if (ret)
	return ret;

    ret = NDBM_open(db);
    if (ret)
	return ret;

    return 0;
}

int
mdb_NDBM_create(MDB **db, const char * filename, int flags, mode_t mode)
{
    *db = malloc(sizeof(**db));
    if (*db == NULL)
	return ENOMEM;

    memset(*db, 0, sizeof(**db));

    (*db)->db = NULL;
    (*db)->name = strdup(filename);
    if ((*db)->name == NULL) {
	free((*db)->name);
	return ENOMEM;
    }
    (*db)->flags = flags;
    (*db)->mode = mode;
    
    (*db)->open = NDBM_open;
    (*db)->close = NDBM_close;
    (*db)->store = NDBM_store;
    (*db)->fetch = NDBM_fetch;
    (*db)->delete = NDBM_delete;
    (*db)->flush = NDBM_flush;

    return 0;
}

#endif /* HAVE_NDBM */
