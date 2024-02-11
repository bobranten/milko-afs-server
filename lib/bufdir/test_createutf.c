/*
 * Copyright (c) 2005 Kungliga Tekniska Högskolan
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

#include <stdio.h>
#include <stdlib.h>
#include <fbuf.h>
#include <fdir.h>

#include <roken.h>
#include <err.h>

RCSID("$Id: test_createutf.c,v 1.3 2006/10/24 16:32:57 tol Exp $");

static void
create_dir(const char *name, int utf8_name, fbuf *the_fbuf, int *fd)
{
    AFSFid dot = { 0, 1, 1 }, dot_dot = { 0, 1, 1 };
    int ret;

    *fd = open(name, O_RDWR | O_CREAT, 0666);
    if (*fd < 0)
	err(1, "open");

    ret = fbuf_create(the_fbuf, *fd, 0, FBUF_WRITE);
    if (ret < 0)
	errx(1, "fbuf_create");

    ret = fdir_mkdir (the_fbuf, dot, dot_dot, utf8_name);
    if (ret < 0)
	errx(1, "fdir_create");
}

static void
create_entry(fbuf *the_fbuf, 
	     const char *name, 
	     const char *raw_utfname, 
	     uint32_t vnode)
{
    AFSFid fid;
    int ret;
    fid.Volume = 0;
    fid.Vnode = vnode;
    fid.Unique = 0;
    
    ret = fdir_creat(the_fbuf, name, raw_utfname, fid);
    if (ret)
	errx(1, "fdir_creat");
}


static void
close_dir(fbuf *the_fbuf, int fd)
{
    int ret;

    ret = fbuf_end(the_fbuf);
    if (ret < 0)
	errx(1, "fbuf_end");

    ret = close(fd);
    if (fd < 0)
	err(1, "close");
}


int
main(int argc, char **argv)
{
    fbuf the_fbuf;
    int fd;

    setprogname (argv[0]);

    create_dir("dir", 0, &the_fbuf, &fd);
    create_entry(&the_fbuf, "name", NULL, 1);
    close_dir(&the_fbuf, fd);
    create_dir("dir-utf8", 1, &the_fbuf, &fd);
    create_entry(&the_fbuf, "name", NULL, 1);
    close_dir(&the_fbuf, fd);

    create_dir("dir-utf8-2", 1, &the_fbuf, &fd);
    create_entry(&the_fbuf, "hörnquist", "hÃ¶rnquist", 1);
    close_dir(&the_fbuf, fd);

    return 0;
}
