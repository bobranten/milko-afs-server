/*
 * Copyright (c) 1995 - 2005 Kungliga Tekniska Högskolan
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

RCSID("$$");

int getcalleraccess_cmd (int argc, char **argv) {
    struct ViceIoctl a_params;
    struct afs_vcxstat2 stat;
    char* path;

    if (argc > 1)
	path = argv[1];
    else
	path = ".";

    a_params.out_size = sizeof(stat);
    a_params.out = (caddr_t)&stat;
    a_params.in_size = 0;
    a_params.in = NULL;

    if (k_pioctl(path, ARLA_VIOC_GETVCXSTATUS2, &a_params, 1) == -1) {
	fserr(PROGNAME, errno, path);
	return errno;
    }

    printf("Caller's access to %s: ", path);

    if (stat.callerAccess & PRSFS_READ)
	printf("r");
    if (stat.callerAccess & PRSFS_LOOKUP)
	printf("l");
    if (stat.callerAccess & PRSFS_INSERT)
	printf("i");
    if (stat.callerAccess & PRSFS_DELETE)
	printf("d");
    if (stat.callerAccess & PRSFS_WRITE)
	printf("w");
    if (stat.callerAccess & PRSFS_LOCK)
	printf("k");
    if (stat.callerAccess & PRSFS_ADMINISTER)
	printf("a");
    printf("\n");

    return 0;
}
