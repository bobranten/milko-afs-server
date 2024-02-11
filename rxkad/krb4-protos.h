/*
 * Copyright (c) 1995-2000 Kungliga Tekniska Högskolan
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

/* @(#)$Id: krb4-protos.h,v 1.3 2005/01/24 06:54:08 lha Exp $ */

#ifndef __KRB4_PROTOS_H
#define __KRB4_PROTOS_H

#define krb4_get_int _rxkad_krb4_get_int
#define krb4_put_int _rxkad_krb4_put_int
#define krb4_get_address _rxkad_krb4_get_address
#define krb4_put_address _rxkad_krb4_put_address
#define krb4_put_string _rxkad_krb4_put_string
#define krb4_get_string _rxkad_krb4_get_string
#define krb4_get_nir _rxkad_krb4_get_nir
#define krb4_put_nir _rxkad_krb4_put_nir
#define krb4_life_to_atime _rxkad_krb4_life_to_atime
#define krb4_atime_to_life _rxkad_krb4_atime_to_life
#define krb4_life_to_time _rxkad_krb4_life_to_time
#define decomp_ticket _rxkad_decomp_ticket

int
krb4_get_int(void *f, u_int32_t *to, int size, int lsb);

int
krb4_put_int(u_int32_t from, void *to, size_t rem, int size);

int
krb4_get_address(void *from, u_int32_t *to);

int
krb4_put_address(u_int32_t addr, void *to, size_t rem);

int
krb4_put_string(const char *from, void *to, size_t rem);

int
krb4_get_string(void *from, char *to, size_t to_size);

int
krb4_get_nir(void *from,
	    char *name, size_t name_len,
	    char *instance, size_t instance_len,
	    char *realm, size_t realm_len);

int
krb4_put_nir(const char *name,
	    const char *instance,
	    const char *realm,
	    void *to,
	    size_t rem);

int
decomp_ticket(KTEXT tkt,	/* The ticket to be decoded */
	      unsigned char *flags, /* Kerberos ticket flags */
	      char *pname,	/* Authentication name */
	      char *pinstance,	/* Principal's instance */
	      char *prealm,	/* Principal's authentication domain */
	      u_int32_t *paddress,/* Net address of entity requesting ticket */
	      unsigned char *session, /* Session key inserted in ticket */
	      int *life,	/* Lifetime of the ticket */
	      u_int32_t *time_sec, /* Issue time and date */
	      char *sname,	/* Service name */
	      char *sinstance,	/* Service instance */
	      DES_cblock *key,	/* Service's secret key (to decrypt the ticket) */
	      DES_key_schedule *schedule); /* The precomputed key schedule */

int krb_get_lrealm (char *r, int n);

char *
krb4_life_to_atime(int life);

int
krb4_atime_to_life(char *atime);

int krb_time_to_life(u_int32_t start, u_int32_t end);

u_int32_t
krb4_life_to_time(u_int32_t start, int life_);

#endif /* __KRB4_PROTOS_H */

