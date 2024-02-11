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

/* $Id: arla-pioctl.h,v 1.8 2007/01/24 22:44:15 lha Exp $ */

#ifndef __ARLA_PIOCT_H
#define __ARLA_PIOCT_H

/* sys/ioctl.h must be included manually before kafs.h */

/*
 */
#define arla_AFSCALL_PIOCTL 20
#define arla_AFSCALL_SETPAG 21

struct arlaViceIoctl {
  caddr_t in, out;
  short in_size;
  short out_size;
};


#define arla_VICEIOCTL(id)  \
	((unsigned int ) _IOW('V', id, struct arlaViceIoctl))
#define arla_ARLAIOCTL(id) \
	((unsigned int ) _IOW('A', id, struct arlaViceIoctl))
#define arla_AFSCOMMONIOCTL(id) \
	((unsigned int ) _IOW('C', id, struct arlaViceIoctl))

/*
 * ioctls
 */

#define ARLA_VIOCCLOSEWAIT		arla_VICEIOCTL(1)
#define ARLA_VIOCABORT			arla_VICEIOCTL(2)
#define ARLA_VIOIGETCELL		arla_VICEIOCTL(3)

/*
 * pioctls
 */

#define ARLA_VIOCSETAL			arla_VICEIOCTL(1)
#define ARLA_VIOCGETAL			arla_VICEIOCTL(2)
#define ARLA_VIOCSETTOK			arla_VICEIOCTL(3)
#define ARLA_VIOCGETVOLSTAT		arla_VICEIOCTL(4)
#define ARLA_VIOCSETVOLSTAT		arla_VICEIOCTL(5)
#define ARLA_VIOCFLUSH			arla_VICEIOCTL(6)
#define ARLA_VIOCSTAT			arla_VICEIOCTL(7)
#define ARLA_VIOCGETTOK			arla_VICEIOCTL(8)
#define ARLA_VIOCUNLOG			arla_VICEIOCTL(9)
#define ARLA_VIOCCKSERV			arla_VICEIOCTL(10)
#define ARLA_VIOCCKBACK			arla_VICEIOCTL(11)
#define ARLA_VIOCCKCONN			arla_VICEIOCTL(12)
#define ARLA_VIOCGETTIME		arla_VICEIOCTL(13)
#define ARLA_VIOCWHEREIS		arla_VICEIOCTL(14)
#define ARLA_VIOCPREFETCH		arla_VICEIOCTL(15)
#define ARLA_VIOCNOP			arla_VICEIOCTL(16)
#define ARLA_VIOCENGROUP		arla_VICEIOCTL(17)
#define ARLA_VIOCDISGROUP		arla_VICEIOCTL(18)
#define ARLA_VIOCLISTGROUPS		arla_VICEIOCTL(19)
#define ARLA_VIOCACCESS			arla_VICEIOCTL(20)
#define ARLA_VIOCUNPAG			arla_VICEIOCTL(21)
#define ARLA_VIOCGETFID			arla_VICEIOCTL(22)
#define ARLA_VIOCWAITFOREVER		arla_VICEIOCTL(23)
#define ARLA_VIOCSETCACHESIZE		arla_VICEIOCTL(24)
#define ARLA_VIOCFLUSHCB		arla_VICEIOCTL(25)
#define ARLA_VIOCNEWCELL		arla_VICEIOCTL(26)
#define ARLA_VIOCGETCELL		arla_VICEIOCTL(27)
#define ARLA_VIOC_AFS_DELETE_MT_PT	arla_VICEIOCTL(28)
#define ARLA_VIOC_AFS_STAT_MT_PT	arla_VICEIOCTL(29)
#define ARLA_VIOC_FILE_CELL_NAME	arla_VICEIOCTL(30)
#define ARLA_VIOC_GET_WS_CELL		arla_VICEIOCTL(31)
#define ARLA_VIOC_AFS_MARINER_HOST	arla_VICEIOCTL(32)
#define ARLA_VIOC_GET_PRIMARY_CELL	arla_VICEIOCTL(33)
#define ARLA_VIOC_VENUSLOG		arla_VICEIOCTL(34)
#define ARLA_VIOC_GETCELLSTATUS		arla_VICEIOCTL(35)
#define ARLA_VIOC_SETCELLSTATUS		arla_VICEIOCTL(36)
#define ARLA_VIOC_FLUSHVOLUME		arla_VICEIOCTL(37)
#define ARLA_VIOC_AFS_SYSNAME		arla_VICEIOCTL(38)
#define ARLA_VIOC_EXPORTAFS		arla_VICEIOCTL(39)
#define ARLA_VIOCGETCACHEPARAMS		arla_VICEIOCTL(40)
#define ARLA_VIOCGETVCXSTATUS		arla_VICEIOCTL(41)
#define ARLA_VIOC_SETSPREFS33		arla_VICEIOCTL(42)
#define ARLA_VIOC_GETSPREFS		arla_VICEIOCTL(43)
#define ARLA_VIOC_GAG			arla_VICEIOCTL(44)
#define ARLA_VIOC_TWIDDLE		arla_VICEIOCTL(45)
#define ARLA_VIOC_SETSPREFS		arla_VICEIOCTL(46)
#define ARLA_VIOC_STORBEHIND		arla_VICEIOCTL(47)
#define ARLA_VIOC_GCPAGS		arla_VICEIOCTL(48)
#define ARLA_VIOC_GETINITPARAMS		arla_VICEIOCTL(49)
#define ARLA_VIOC_GETCPREFS		arla_VICEIOCTL(50)
#define ARLA_VIOC_SETCPREFS		arla_VICEIOCTL(51)
#define ARLA_VIOC_FLUSHMOUNT		arla_VICEIOCTL(52)
#define ARLA_VIOC_RXSTATPROC		arla_VICEIOCTL(53)
#define ARLA_VIOC_RXSTATPEER		arla_VICEIOCTL(54)

#define ARLA_VIOC_GETRXKCRYPT		arla_VICEIOCTL(55) /* 48 in some implementations */
#define ARLA_VIOC_SETRXKCRYPT		arla_VICEIOCTL(56) /* with cryptosupport in afs */

/* arla specific */

#define ARLA_VIOC_FPRIOSTATUS		arla_VICEIOCTL(57) /* arla: set file prio */
#define ARLA_VIOC_FHGET			arla_VICEIOCTL(58) /* arla: fallback getfh */
#define ARLA_VIOC_FHOPEN		arla_VICEIOCTL(59) /* arla: fallback fhopen */
#define ARLA_VIOC_NNPFSDEBUG		arla_VICEIOCTL(60) /* arla: controls nnpfsdebug */
#define ARLA_VIOC_ARLADEBUG		arla_VICEIOCTL(61) /* arla: controls arla debug */
#define ARLA_VIOC_AVIATOR		arla_VICEIOCTL(62) /* arla: debug interface */
#define ARLA_VIOC_NNPFSDEBUG_PRINT	arla_VICEIOCTL(63) /* arla: print nnpfs status */
#define ARLA_VIOC_CALCULATE_CACHE	arla_VICEIOCTL(64) /* arla: force cache check */
#define ARLA_VIOC_BREAKCALLBACK		arla_VICEIOCTL(65) /* arla: break callback */

#define ARLA_VIOC_PREFETCHTAPE		arla_VICEIOCTL(66) /* MR-AFS prefetch from tape */
#define ARLA_VIOC_RESIDENCY_CMD		arla_VICEIOCTL(67) /* generic MR-AFS cmds */
#define ARLA_VIOC_GETVCXSTATUS2      	arla_VICEIOCTL(69)  /* vcache statistics */


#define ARLA_AIOC_STATISTICS		arla_ARLAIOCTL(1) /* arla: fetch statistics */
#define ARLA_AIOC_PTSNAMETOID		arla_ARLAIOCTL(2) /* arla: pts name to id */
#define ARLA_AIOC_GETCACHEPARAMS	arla_ARLAIOCTL(3) /* arla: get cache params */
#define ARLA_AIOC_GETPREFETCH		arla_ARLAIOCTL(4) /* arla: get prefetch value */
#define ARLA_AIOC_SETPREFETCH		arla_ARLAIOCTL(5) /* arla: set prefetch value */
#define ARLA_AFSCOMMONIOC_GKK5SETTOK	arla_ARLAIOCTL(6) /* gk k5 set token */
#define ARLA_AFSCOMMONIOC_GKLISTTOK	arla_ARLAIOCTL(7) /* gk list token */
#define ARLA_AIOC_SETCACHEPARAMS	arla_ARLAIOCTL(8) /* arla: set cache params */
#define ARLA_AIOC_CONNECTMODE	        arla_ARLAIOCTL(9)

#define ARLA_AFSCOMMONIOC_NEWALIAS	arla_AFSCOMMONIOCTL(1) /* common: ... */
#define ARLA_AFSCOMMONIOC_LISTALIAS	arla_AFSCOMMONIOCTL(2) /* common: ... */

#define ARLA_VIOCGETTOK2		arla_AFSCOMMONIOCTL(7)
#define ARLA_VIOCSETTOK2 		arla_AFSCOMMONIOCTL(8)

#define ARLA_TOKEN_TYPE_NULL 0
/* secindex 1 was used for vab */
#define ARLA_TOKEN_TYPE_KAD  2
/* secindex 3 was used for broken rxkad cryptall */
#define ARLA_TOKEN_TYPE_K5   4
#define ARLA_TOKEN_TYPE_GK   5

/*
 * GETCELLSTATUS flags
 */

#define arla_CELLSTATUS_PRIMARY		0x01 /* this is the `primary' cell */
#define arla_CELLSTATUS_SETUID		0x02 /* setuid honored for this cell */
#define arla_CELLSTATUS_OBSOLETE_VL	0x04 /* uses obsolete VL servers */

/*
 * ARLA_AIOC_CONNECTMODE arguments
 */

#define arla_CONNMODE_PROBE 0
#define arla_CONNMODE_CONN 1
#define arla_CONNMODE_FETCH 2
#define arla_CONNMODE_DISCONN 3
#define arla_CONNMODE_PARCONNECTED 4
#define arla_CONNMODE_CONN_WITHCALLBACKS 5

/*
 * The struct for VIOC_FPRIOSTATUS
 */

#define arla_FPRIO_MAX 100
#define arla_FPRIO_MIN 0
#define arla_FPRIO_DEFAULT arla_FPRIO_MAX

#define arla_FPRIO_GET 0
#define arla_FPRIO_SET 1
#define arla_FPRIO_GETMAX 2
#define arla_FPRIO_SETMAX 3

struct arla_vioc_fprio {
    int16_t cmd;
    int16_t prio;
    int32_t Cell;
    int32_t Volume;
    int32_t Vnode;
    int32_t Unique;
};


/*
 * Flags for VIOCCKSERV
 */

#define arla_CKSERV_DONTPING     1
#define arla_CKSERV_FSONLY       2

#define arla_CKSERV_MAXSERVERS   16 /* limitation of VIOCCKSERV number of 
				  returned servers */

/* 
 *  for AIOC_STATISTICS
 */

#define arla_STATISTICS_OPCODE_LIST 0
#define arla_STATISTICS_OPCODE_GETENTRY 1

#define arla_STATISTICS_REQTYPE_FETCHSTATUS 1
#define arla_STATISTICS_REQTYPE_FETCHDATA 2
#define arla_STATISTICS_REQTYPE_BULKSTATUS 3
#define arla_STATISTICS_REQTYPE_STOREDATA 4
#define arla_STATISTICS_REQTYPE_STORESTATUS 5

/* 
 *  for AIOC_GETCACHEPARAMS
 */

#define arla_GETCACHEPARAMS_OPCODE_HIGHBYTES		1
#define arla_GETCACHEPARAMS_OPCODE_USEDBYTES		2
#define arla_GETCACHEPARAMS_OPCODE_LOWBYTES		3
#define arla_GETCACHEPARAMS_OPCODE_HIGHVNODES		4
#define arla_GETCACHEPARAMS_OPCODE_USEDVNODES		5
#define arla_GETCACHEPARAMS_OPCODE_LOWVNODES		6

/* 
 *  for AIOC_SETCACHEPARAMS
 */

#define arla_SETCACHEPARAMS_OPCODE_BYTES		1
#define arla_SETCACHEPARAMS_OPCODE_VNODES		2


struct arla_ClearToken {
  int32_t AuthHandle;
  char HandShakeKey[8];
  int32_t ViceId;
  int32_t BeginTimestamp;
  int32_t EndTimestamp;
};

/*
 * for ARLA_VIOC_GETVCXSTATUS2
 */

struct afs_vcxstat2 {
    int32_t callerAccess;
    int32_t cbExpires;
    int32_t anyAccess;
    char mvstat;
};


#endif /* __ARLA_PIOCT_H */
