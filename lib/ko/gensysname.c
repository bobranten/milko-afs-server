/*
 * Copyright (c) 1998 - 2002, 2003 - 2006 Kungliga Tekniska Högskolan
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

#include "ko_locl.h"
#include <fnmatch.h>
#include <getarg.h>

RCSID("$Id: gensysname.c,v 1.48 2006/03/23 09:51:03 tol Exp $");

typedef int (*test_sysname)(void);
typedef void (*gen_sysname)(char*, size_t, const char*, 
			    const char*, const char*);

struct sysname {
    const char *sysname;
    const char *cpu;
    const char *vendor;
    const char *os;
    test_sysname atest;
    gen_sysname gen;
};

/*
 * return what we think the libc version is
 */

static int
linux_glibc_version(void)
{
#if defined(__GLIBC__) && defined(__GLIBC_MINOR__) && 0
    return __GLIBC__ * 10 + __GLIBC_MINOR__;
#else
    struct stat sb;
    return stat("/lib/libc.so.6", &sb) ? 5 : 6;
#endif
}

static void
linux_gen_sysname(char *buf,
		  size_t len,
		  const char *cpu,
		  const char *vendor,
		  const char *os)
{
    const char *machine = cpu;
    int libc = linux_glibc_version();

    if (!fnmatch("i*86*", cpu, 0))
	machine = "i386";
    else if (!fnmatch("x86_64", cpu, 0))
	machine = "amd64";
    else if (!fnmatch("sparc*", cpu, 0))
	machine = "sparc";
    else if (!fnmatch("powerpc*", cpu, 0))
	machine = "ppc";
	     
    snprintf(buf, len, "%s_linux%d", machine, libc);
}

static void
darwin_gen_sysname(char *buf,
		   size_t len,
		   const char *cpu,
		   const char *vendor,
		   const char *os)
{
    const char *machine = cpu;
    
    if (!fnmatch("i*86*", cpu, 0))
	machine = "i386";
    else if (!fnmatch("Power*", cpu, 0))
	machine = "ppc";
	     
    snprintf(buf, len, "%s_macosx", machine);
}

static void
osf_gen_sysname(char *buf, 
		size_t len,
		const char *cpu, 
		const char *vendor, 
		const char *os)
{
    int minor, major, nargs;
    char patch;
    nargs = sscanf(os, "osf%d.%d%c", &major, &minor, &patch);
    if(nargs == 3) {
	snprintf(buf, len, "alpha_osf%d%d%c", major, minor, patch);
    } else if(nargs == 2) {
	snprintf(buf, len, "alpha_osf%d%d", major, minor);
    } else {
	snprintf(buf, len, "alpha_osf");
    }
}


/*
 * generic function for generating sysnames for *BSD systems.  the
 * sysname is written into `buf' (of length `len') based on `cpu,
 * vender, os'.
 */

#ifdef HAVE_SYS_UTSNAME_H
static void
generic_sysname(char *buf, 
		size_t len,
		const char *cpu, 
		const char *vendor, 
		const char *os)
{
    struct utsname uts;
    int major, minor;
    const char *name;
    if(uname(&uts) < 0) {
	warn("uname");
	strlcpy(buf, "bsdhost", len);
	return;
    }

    cpu = uts.machine;

    if (strcmp(uts.sysname, "Linux") == 0)
	return linux_gen_sysname(buf, len, cpu, vendor, os);

    if (strcmp(uts.sysname, "Darwin") == 0)
	return darwin_gen_sysname(buf, len, cpu, vendor,
				  uts.sysname);

    if(strcmp(uts.sysname, "FreeBSD") == 0)
	name = "fbsd";
    else if(strcmp(uts.sysname, "NetBSD") == 0)
	name = "nbsd";
    else if(strcmp(uts.sysname, "OpenBSD") == 0)
	name = "obsd";
    else if(strcmp(uts.sysname, "BSD/OS") == 0)
	name = "bsdi";
    else if(strcmp(uts.sysname, "DragonFly") == 0)
	name = "dfly";
    else if(strcmp(uts.sysname, "SunOS") == 0) {
	name = "";
	if (!fnmatch("i*86*", cpu, 0))
	    cpu = "sunx86";
	else if (!strcmp("sun4u", cpu) || !fnmatch("sparc*", cpu, 0))
	    cpu = "sun4x";
    } else
	name = "bsd";

    /* this is perhaps a bit oversimplified */
    if(sscanf(uts.release, "%d.%d", &major, &minor) == 2)
	snprintf(buf, len, "%s_%s%d%d", cpu, name, major, minor);
    else
	snprintf(buf, len, "%s_%s", cpu, name);
}
#endif

/* 
 * HELP:
 *
 * Add your sysname to the struct below, it's searched from top
 * to bottom, first match wins.
 *
 * ? will match any character
 * * will match any sequence of characters
 */

struct sysname sysnames[] = {
    /* use uname if we can */
#ifdef HAVE_SYS_UTSNAME_H
    { "",	      "*",    "*", "*",  NULL, &generic_sysname },
#endif
    { "sun4x_551",    "sparc*", "*", "solaris2.5.1*", NULL },
    { "i386_nt35",    "i*86*", "*", "cygwin*", NULL },
    { "",	      "alpha*",    "*", "*osf*",  NULL, &osf_gen_sysname },
    {NULL}
};

static void
find_sysname(char *buf, 
	     size_t len,
	     const char *cpu, 
	     const char *vendor, 
	     const char *os)
{
    struct sysname *sysname = sysnames;
    int found = 0;

    while (sysname->sysname && !found) {
	if (!fnmatch(sysname->cpu, cpu, 0) &&
	    !fnmatch(sysname->vendor, vendor, 0) &&
	    !fnmatch(sysname->os, os, 0) &&
	    (sysname->atest == NULL || ((*(sysname->atest))()))) {
	    
	    found = 1;
	    if(sysname->gen != NULL)
		(*sysname->gen)(buf, len, cpu, vendor, os);
	    else {
		strlcpy(buf, sysname->sysname, len);
	    }
	}
	sysname++;
    }

    /* XXX need some better here? */
    if (!found) {
	fprintf(stderr, "our host was not found using generic\n"); 
	strlcpy(buf, "arlahost", len);
    }
}

/*
 * Our exported interface.
 */

const char *
arla_getsysname(void)
{
    static char sn[64];

    if (sn[0] == '\0')
        find_sysname(sn, sizeof(sn), ARLACPU, ARLAVENDOR, ARLAOS);
    if (sn[0] == '\0')
	return "arlahost";
    
    return sn;
}

/*
 * Used for testing purposes.
 */

int
_arla_getsysname_hint(char *buf, 
		      size_t len,
		      const char *cpu, 
		      const char *vendor, 
		      const char *os)
{
    find_sysname(buf, len, cpu, vendor, os);
    if (buf[0] == '\0')
	return -1;
    return 0;
}
