AC_REVISION($Revision: 1.759 $)dnl
AC_PREREQ([2.71])
AC_INIT([milko],[0.90b],[arla-drinkers@stacken.kth.se])
arla_CANONICAL
arla_OPENSSL_COMPAT
AM_INIT_AUTOMAKE([foreign no-dependencies 1.9])
AM_MAINTAINER_MODE
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_MACRO_DIRS([cf])

dnl
dnl definitions
dnl

CFLAGS="-g ${CFLAGS}"

if test "$AS" = ""; then
  AS=as
fi
AC_SUBST(AS)dnl

AC_PROG_CC
AC_PROG_CC_FLAGS
AC_PROG_CPP

# This may be overridden using --prefix=/usr to configure
AC_PREFIX_DEFAULT(/usr/local)

CANONICAL_HOST=$host
AC_SUBST(CANONICAL_HOST)

dnl
dnl this is needed to run the configure tests against glibc
dnl
AC_DEFINE([_GNU_SOURCE], 1,
	[Define to enable extensions on glibc-based systems such as Linux.])

AC_OBJEXT
AC_EXEEXT

AC_PROG_LEX([noyywrap])
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_YACC
dnl remove ifdef when we use 2.54
m4_ifdef([AC_PROG_EGREP], [AC_PROG_EGREP],)

AC_CHECK_PROGS(SYMORDER, symorder, :)
AC_CHECK_PROGS(MAKEINFO, makeinfo, :)
AC_CHECK_PROGS(DVI2PS, dvi2ps, :)
AC_CHECK_PROGS(TEXI2DVI, texi2dvi, :)
AC_CHECK_PROGS(TEXI2PDF, texi2pdf, :)
AC_CHECK_PROGS(TEXI2HTML, texi2html, :)
AC_CHECK_PROGS(DVIPS, dvips, :)
AC_CHECK_PROGS(chmod, chmod, :)
AC_PATH_PROGS(GUILE_GTK, guile-gtk, /bin/false)

case $target_os in
    linux*)
    	force_large_file=yes ;;
    *)
    	force_large_file=no ;;
esac

if test "${force_large_file}" = "yes" ; then
    AC_DEFINE(_LARGEFILE_SOURCE, 1,
        [Define if you have largefile support])
    AC_DEFINE(_LARGEFILE64_SOURCE, 1,
        [Define if you have largefile support])
    AC_DEFINE(_FILE_OFFSET_BITS, 64,
        [Define how big file offset is in your OS. 32/64 are valid values.])
fi


dnl 
dnl See if there is any X11 present
dnl
KRB_CHECK_X
AM_CONDITIONAL(X11, test X"$no_x" != X"yes")

#AC_FUNC_NTOHL

# what kind of lwp

LWP_PROCESS="process.o"
LWP_C="lwp_asm.c"
LWP_O="lwp_asm.o"
LWP_H="lwp_asm.h"
PLWP_LIB_FLAGS=""
PLWP_INC_FLAGS=""

AC_ARG_WITH(pthreads,
[  --with-pthreads=dir       compile liblwp as pthreads wrapper using pthreads in dir],
# always use pthreads
[if test "$with_pthreads" = "windows"; then
  with_pthreads=win
elif test "$with_pthreads" != "no"; then
  LWP_PROCESS=""
  LWP_C="plwp.c"
  LWP_O="plwp.o"
  LWP_H="plwp.h"
  PLWP_INC_FLAGS="-DPTHREADS_LWP"
  if test "$with_pthreads" != "yes"; then
    AC_FIND_FUNC_NO_LIBS(pthread_create, pthread,,,[-L${with_pthreads}/lib])
    PLWP_LIB_FLAGS="-L${with_pthreads}/lib $LIB_pthread_create"
    PLWP_INC_FLAGS="-I${with_pthreads}/include -DPTHREADS_LWP"
  else
    AC_FIND_FUNC_NO_LIBS(pthread_create, pthread)
    PLWP_LIB_FLAGS="$LIB_pthread_create"
    PLWP_INC_FLAGS="-DPTHREADS_LWP"
  fi
  AC_FIND_FUNC_NO_LIBS(sched_yield, pthread,,,[${PLWP_LIB_FLAGS}])
fi],[with_pthreads=no])

# always use pthreads
LWP_PROCESS=""
LWP_C="plwp.c"
LWP_O="plwp.o"
LWP_H="plwp.h"
PLWP_INC_FLAGS="-DPTHREADS_LWP"

dnl ydr need special flags
AC_SUBST(YDR_CPPFLAGS)

dnl are we dragonfly
AM_CONDITIONAL(DFLY, expr "$target_os" : "dragonfly.*" >/dev/null 2>&1)

dnl are we freebsd5
AM_CONDITIONAL(FBSD5, test X"$FREEBSD5" = Xyes)

dnl are we linux-ia64
AM_CONDITIONAL(LINUX_IA64, test X"$LINUX_IA64" = Xyes)

AC_SUBST(BSD_WERROR)

AM_CONDITIONAL(DCE, false)

# Check where to store partitions

AC_ARG_WITH(milkopart,
[  --with-milkopart=dir    location of milko partitions (default /)],
[MILKO_ROOT='-DMILKO_ROOT=\"$withval\"'],
[MILKO_ROOT='-DMILKO_ROOT=\"/\"'])

dnl
dnl If we want to use pthreads
dnl

AC_SUBST(LWP_PROCESS)
AC_SUBST(PLWP_LIB_FLAGS)
AC_SUBST(PLWP_INC_FLAGS)
AC_SUBST(LWP_C)
AC_SUBST(LWP_O)
AC_SUBST(LWP_H)

AC_TYPE_MSGHDR
AC_TYPE_IOVEC

dnl AC_TYPE_MODE_T
dnl AC_CHECK_TYPE(nlink_t, int)

dnl
dnl Tests for readline/editline
dnl

rk_TEST_PACKAGE(readline,
[#include <stdio.h>
 #include <readline.h>],-lreadline,,, READLINE)

if test "$with_readline" = "no"; then
	AC_FIND_FUNC_NO_LIBS(tgetent, termcap ncurses curses)
	if test "$ac_cv_funclib_tgetent" = "no"; then
		AC_MSG_ERROR([Could not find tgetent, needed by edit/editline])
	fi
	AC_FIND_FUNC_NO_LIBS(readline, editline edit readline, [], [], [$LIB_tgetent])
	if test "$ac_cv_func_readline" = "no"; then
		AC_FIND_FUNC_NO_LIBS(el_init, edit, [], [], [$LIB_tgetent])
		if test "$ac_cv_func_el_init" = yes; then
			editline_OBJS=edit_compat.o
			LIB_readline='-L'`pwd`'/lib/editline -leditline '"$LIB_el_init";
			INCLUDE_readline='-I'`pwd`'/lib/editline -I$(top_srcdir)/lib/editline'
		else
			editline_OBJS="editline.o complete.o sysunix.o"
			LIB_readline='-L'`pwd`'/lib/editline -leditline'
			INCLUDE_readline='-I'`pwd`'/lib/editline -I$(top_srcdir)/lib/editline'
		fi
	fi
fi
AC_DEFINE(HAVE_READLINE, 1, [define if you have a function readline])

LIB_readline_ac="$LIB_readline $LIB_tgetent"
LIB_readline="$LIB_readline \$(LIB_tgetent)"

dnl
dnl
dnl

AC_ARG_ENABLE(mmap,
[  --disable-mmap          don't use mmap],
[if test "$enableval" = "no"; then
  ac_cv_func_mmap_fixed_mapped=no
 fi])

aix_dynamic_afs=yes
AC_ARG_ENABLE(dynamic-afs,
[  --disable-dynamic-afs   don't use loaded AFS library with AIX],[
if test "$enableval" = "no"; then
	aix_dynamic_afs=no
fi
])

syscallhack=yes
AC_ARG_ENABLE(linux-syscall,
	AS_HELP_STRING([--disable-linux-syscall],[don't modify the syscall table on linux]),
[if test "$enableval" = "no" ; then
	syscallhack=no
fi])
if test "$syscallhack" = "yes"; then
	AC_DEFINE(SYSCALLHACK, 1, [define to enable syscall table modification on linux])
fi

grouppags=yes
AC_ARG_ENABLE(linux-group-pags,
	AS_HELP_STRING([--disable-linux-group-pags],[don't use groups for pags on linux]),
[if test "$enableval" = "no" ; then
	grouppags=no
fi])
if test "$grouppags" = "yes"; then
	AC_DEFINE(GROUPPAGS, 1, [define to enable groups-based pags on linux])
fi

AC_CHECK_LFS
AC_CHECK_GLIBC

AC_SUBST(LIB_readline)
AC_SUBST(INCLUDE_readline)
AC_SUBST(editline_OBJS)dnl
if test "$editline_OBJS" != ""; then
  editline_dir=editline
fi
AC_SUBST(editline_dir)

AC_SUBST(GCC)
AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(KERNEL_INCLUDE)
AC_SUBST(KERNEL_SRCS)
AC_SUBST(KERNEL_HDRS)
AC_SUBST(KERNEL_CPPFLAGS)
AC_SUBST(KERNEL_CFLAGS)
AC_SUBST(KERNEL_LD_FLAGS)
AC_SUBST(KERNEL_CC)
AC_SUBST(KERNEL_LD)
AC_SUBST(MODLOAD)
AC_SUBST(MODUNLOAD)
AC_SUBST(MODULE)
AC_SUBST(RXKAD_LIBS)
AC_SUBST(MILKO_ROOT)
AC_FUNC_MMAP

dnl
dnl Various checks for headers and their contents
dnl

AC_CHECK_HEADERS([				\
		arpa/inet.h			\
		arpa/nameser.h			\
		dbm.h				\
		db.h				\
		dirent.h			\
		err.h				\
		errno.h				\
		elflib/nlist.h			\
		fcntl.h				\
		fnmatch.h			\
		gdbm/ndbm.h			\
		glob.h				\
		ifaddrs.h			\
		libelf/nlist.h			\
		limits.h			\
		machine/asm.h			\
		machine/regdef.h		\
		mach/alpha/asm.h		\
		ndbm.h				\
		netdb.h				\
		net/if_dl.h			\
		net/if_types.h			\
		netinet/in6.h			\
		netinet/in6_machtypes.h		\
		netinet/in.h			\
		nlist.h				\
		openssl/ui.h			\
		pwd.h				\
		regdef.h			\
		rpcsvc/dbm.h			\
		shadow.h			\
		stdint.h			\
		sys/attr.h			\
		sys/bitypes.h			\
		sys/cdefs.h			\
		sys/dir.h			\
		sys/exec.h			\
		sys/file.h			\
		sys/ioccom.h			\
		sys/ioctl.h			\
		sys/inttypes.h			\
		sys/kauth.h			\
		sys/libkern.h			\
		sys/lkm.h			\
		sys/lock.h			\
		sys/mkdev.h			\
		sys/mman.h			\
		sys/module.h			\
		sys/mutex.h			\
		sys/param.h			\
		sys/prctl.h			\
		sys/priv.h			\
		sys/poll.h			\
		sys/pool.h			\
		sys/queue.h			\
		sys/resource.h			\
		sys/sa.h			\
		sys/select.h			\
		sys/selinfo.h			\
		sys/socket.h			\
		sys/sockio.h			\
		sys/stat.h			\
		sys/statvfs.h			\
		sys/statfs.h			\
		sys/stdint.h			\
		sys/sysent.h			\
		sys/sysproto.h			\
		sys/systm.h			\
		sys/time.h			\
		sys/types.h			\
		sys/ubc.h			\
		sys/utsname.h			\
		sys/vfs.h			\
		sys/vfs_proto.h			\
		sys/vnode.h			\
		sys/wait.h			\
		vis.h				\
		unistd.h			\
		winsock.h			\
])

AC_CHECK_HEADERS([net/if.h], , , [AC_INCLUDES_DEFAULT
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif])

AC_CHECK_HEADERS(netinet6/in6_var.h, , , [AC_INCLUDES_DEFAULT
#ifdef HAVE_NETINET6_IN6_H
#include <netinet6/in6.h>
#endif
])

AC_CHECK_HEADERS([			\
		sys/proc.h		\
		sys/mount.h		\
		sys/signalvar.h		\
		sys/user.h		\
], , , [AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
])

AC_CHECK_HEADERS([			\
		sys/syscallargs.h	\
], , , [AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_SA_H
#include <sys/sa.h>
#endif
])

AC_CHECK_HEADERS([resolv.h], , , [AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif
])

#AC__EXTENSIONS__

dnl
dnl These need to before roken checks
dnl

AM_CONDITIONAL(have_err_h, test "$ac_cv_header_err_h" = yes)
AM_CONDITIONAL(have_fnmatch_h, test "$ac_cv_header_fnmatch_h" = yes)
AM_CONDITIONAL(have_ifaddrs_h, test "$ac_cv_header_ifaddrs_h" = yes)
AM_CONDITIONAL(have_vis_h, test "$ac_cv_header_vis_h" = yes)
AM_CONDITIONAL(have_glob_h, test "$ac_cv_header_glob_h" = yes)
AM_CONDITIONAL(do_roken_rename, true)

dnl
dnl and this is for kernel header files
dnl

save_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${KERNEL_CPPFLAGS}"

CPPFLAGS="${save_CPPFLAGS}"


dnl
dnl Check for endian-ness
dnl
KRB_C_BIGENDIAN

dnl
dnl check for const
dnl

AC_C_CONST

dnl
dnl Check for inline
dnl

AC_C_INLINE

dnl
dnl Check for __FUNCTION__
dnl

AC_C___FUNCTION__

dnl
dnl Check for __attribute__
dnl

AC_C___ATTRIBUTE__

dnl
dnl roken stuff
dnl

AC_ENABLE_SHARED(no)
LT_INIT

rk_DB

#
# It is said that the kerberos junk needs to be after ac_check_roken,
# but I didn't notice that, so we include some to have a good place
# to check for existing roken in 
#

AC_CHECK_KERBEROS(54auto)
AC_CHECK_KAFS
#end krb, continue roken stuff

AC_CHECK_ROKEN()

# oh, and put sl here, too
AC_CHECK_SL
# done!

if test "X$ac_cv_arla_with_roken" = "Xyes" ; then
   rk_ROKEN(lib/roken)
   #LIB_roken="\$(top_builddir)/lib/vers/libvers.la $LIB_roken"
else
  dnl inlined parts of rk_ROKEN, seems to be needed by sl and rx sometimes.
  AC_BROKEN([					\
	err					\
	errx					\
	strdup					\
	strtok_r				\
	strupr					\
	warn					\
	warnx					\
  ])
fi
AM_CONDITIONAL(ARLA_LOCAL_ROKEN, test X$ac_cv_arla_with_roken = Xyes)

rk_CHECK_MAN

if test "X$ac_cv_found_krb5" = "Xyes" ; then
  RXKAD_LIBS_extra="$RXKAD_LIBS_extra $KRB5_LIB_FLAGS"
fi

if test "X$RXKAD_LIBS_extra" != "X" ; then
  RXKAD_LIBS="\$(top_builddir)/rxkad/librxkad.la $RXKAD_LIBS_extra"
fi
AM_CONDITIONAL(RXKAD, test "X$RXKAD_LIBS_extra" != "X")

dnl AC_TYPE_KRB_PRINCIPAL
dnl AC_FUNC_KRB_GET_ERR_TEXT
dnl AC_FUNC_KRB_GET_DEFAULT_TKT_ROOT
dnl AC_FUNC_KRB_GET_DEFAULT_PRINCIPAL
dnl AC_FUNC_KRB_KDCTIMEOFDAY
AC_FUNC_KAFS_SETTOKEN_RXKAD

#
# check for tf_util
#

dnl AC_FIND_FUNC_NO_LIBS2(tf_init, "", , , [$KRB4_LIB_FLAGS])
dnl 
dnl if test "$ac_cv_func_tf_init" = yes; then
dnl 	APPL_SUBDIRS="$APPL_SUBDIRS kalog"
dnl fi
dnl 

dnl
dnl Check for strange operating systems that you need to handle differently
dnl

AC_KRB_SYS_AIX

if test "$krb_cv_sys_aix" = yes ;then
	if test "$aix_dynamic_afs" = yes; then
		AFS_EXTRA_OBJS='$(srcdir)/afsl.exp dlfcn.o'
		AFS_EXTRA_LIBS=afslib.so
		# this works differently in AIX <=3 and 4
		if test `uname -v` = 4 ; then
			AFS_EXTRA_LD="-bnoentry"
		else
			AFS_EXTRA_LD="-e _nostart"
		fi
		AFS_EXTRA_DEFS=
		AIX_EXTRA_KAFS="-lld"
	else
		AFS_EXTRA_OBJS='$(srcdir)/afsl.exp afslib.o'
		AFS_EXTRA_LIBS=
		AFS_EXTRA_DEFS='-DSTATIC_AFS_SYSCALLS'
		AIX_EXTRA_KAFS=
	fi
	AC_SUBST(AFS_EXTRA_OBJS)dnl
	AC_SUBST(AFS_EXTRA_LIBS)dnl
	AC_SUBST(AFS_EXTRA_LD)dnl
	AC_SUBST(AFS_EXTRA_DEFS)dnl
	AC_SUBST(AIX_EXTRA_KAFS)dnl
fi

dnl
dnl Check for struct winsize
dnl

AC_KRB_STRUCT_WINSIZE
 
dnl
dnl Various checks for libraries and their contents
dnl

AC_FIND_FUNC(syslog, syslog)

dnl
dnl System V is have misplaced the socket routines, should really be in libc
dnl

AC_FIND_FUNC(socket, socket)
AC_FIND_FUNC(gethostbyname, nsl)
AC_FIND_FUNC(gethostbyname2, inet6 ip6)

dnl we don't actually use v6 functions in this code, but some generic
dnl functions such as getnameinfo et al can reside in libraries that are
dnl found by this macro

AC_KRB_IPV6

AC_FIND_FUNC(res_search, resolv,
[
#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif
#ifdef HAVE_RESOLV_H
#include <resolv.h>
#endif
],
[0,0,0,0,0])

AC_FIND_FUNC(res_init, resolv,
[
#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif
#ifdef HAVE_RESOLV_H
#include <resolv.h>
#endif
],
[])

AC_FIND_FUNC(dn_expand, resolv,
[
#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif
#ifdef HAVE_RESOLV_H
#include <resolv.h>
#endif
],
[0,0,0,0,0])

AC_BROKEN_SNPRINTF

AC_CHECK_FUNCS(strtoll strtoq getrusage thr_yield)

AC_EGREP_HEADER(sigaction, signal.h,
	AC_DEFINE(HAVE_POSIX_SIGNALS, 1, [define if you have sigaction]))

save_CPPFLAGS="${CPPFLAGS}"
AC_HAVE_TYPES([int8_t int16_t int32_t int64_t])
AC_HAVE_TYPES([int16 int32])
AC_HAVE_TYPES([u_int8_t u_int16_t u_int32_t u_int64_t])
AC_HAVE_TYPES([uint8_t uint16_t uint32_t uint64_t])
AC_HAVE_TYPES([u_int16 u_int32])
AC_HAVE_TYPES([bool ssize_t])
AC_HAVE_TYPES([intptr_t])
AC_HAVE_TYPES([off64_t])
AC_CHECK_HEADERS([ktypes.h com_err.h et/com_err.h])
COMERR_CPPFLAGS="$CPPFLAGS"
AC_SUBST(COMERR_CPPFLAGS)
CPPFLAGS="${save_CPPFLAGS}"


dnl
dnl check for old libkafs
dnl

dnl AC_FIND_FUNC_NO_LIBS2(krb_afslog_uid, "", , , [$KAFS_LIBS $KRB4_LIB_FLAGS])


AC_CHECK_FUNC(localtime_r,[
AC_DEFINE([HAVE_LOCALTIME_R], 1, [define if there exists a localtime_r])])

#
# libroken
#

AC_CHECK_FUNCS(getfh fhopen)

AC_CHECK_FUNCS(getattrlist setattrlist)

AC_CHECK_FUNCS(statvfs)

rk_CHECK_VAR(optreset,[#include <stdlib.h>])dnl

dnl
dnl prototypes
dnl

AC_NEED_PROTO([
#include <sys/types.h> 
#include <sys/time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
], 
select)

AC_DIRENT_SYS_DIR_H

AC_HAVE_STRUCT_FIELD(struct dirent, d_type,
[#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <dirent.h>
])

dnl
dnl Check for sa_len in sys/socket.h
dnl

AC_HAVE_STRUCT_FIELD(struct sockaddr,
sa_len,
[#include <sys/types.h>
#include <sys/socket.h>])

dnl
dnl Check for sin_len
dnl

AC_HAVE_STRUCT_FIELD(struct sockaddr_in,
sin_len,
[#include <sys/types.h>
#include <sys/socket.h>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif])

dnl
dnl check for strange as stuff on solaris
dnl

case "$target_os" in
solaris*)
#  AC_CACHE_CHECK(if as supports .register, ac_cv_prog_as_register,[
  cat > conftest.s <<EOF
	.register	%g2, #scratch
	.register	%g3, #scratch
	.register	%g6, #scratch
	.register	%g7, #scratch
EOF
  if $AS conftest.s >/dev/null 2>&1; then
    ac_cv_prog_as_register=yes
  else
    ac_cv_prog_as_register=no
  fi
  rm -f conftest.s a.out
#])
  if test "$ac_cv_prog_as_register" = "yes"; then
    AC_DEFINE([PROG_AS_UNDERSTANDS_REGISTER], 1,
	[define this if your as understands .register])
  fi
esac

dnl
dnl Global kernel checks
dnl

AC_CHECK_KERNEL_FUNCS(strlcpy)

#
# Libtool vs automake stuff
#

LTLIBOBJS=`echo "$LIB@&t@OBJS" | sed 's/\.o/\.lo/g'`
AC_SUBST(LTLIBOBJS)

AH_BOTTOM([#ifdef ROKEN_RENAME
#include "roken_rename.h"
#endif])

# Almost done....

AC_CONFIG_FILES([Makefile			\
	conf/Makefile				\
	include/Makefile			\
	lwp/Makefile				\
	rx/Makefile				\
	rxdef/Makefile				\
	rxgk/Makefile				\
	rxkad/Makefile				\
	ydr/Makefile				\
	lib/Makefile				\
	lib/arla/Makefile			\
	lib/bufdir/Makefile			\
	lib/editline/Makefile			\
	lib/ko/Makefile				\
	lib/roken/Makefile			\
	lib/sl/Makefile				\
	lib/util/Makefile			\
	lib/vers/Makefile			\
	milko/Makefile 				\
	milko/lib/Makefile 			\
	milko/lib/dpart/Makefile		\
	milko/lib/mdb/Makefile			\
	milko/lib/mlog/Makefile			\
	milko/lib/msecurity/Makefile		\
	milko/lib/ropa/Makefile			\
	milko/lib/vld/Makefile			\
	milko/lib/voldb/Makefile		\
	milko/lib/vstatus/Makefile		\
	milko/appl/Makefile			\
	milko/appl/bootstrap/Makefile		\
	milko/appl/sked/Makefile		\
	milko/bos/Makefile			\
	milko/fs/Makefile			\
	milko/pts/Makefile			\
	milko/vldb/Makefile			\
	appl/Makefile				\
	appl/arlalib/Makefile			\
	appl/afstool/Makefile			\
	appl/afsutils/Makefile			\
	appl/amon/Makefile			\
	appl/bos/Makefile			\
	appl/fs/Makefile			\
	appl/kalog/Makefile			\
	appl/perf/Makefile			\
	appl/pts/Makefile			\
	appl/rxgklog/Makefile			\
	appl/udebug/Makefile			\
	appl/vos/Makefile])
AC_OUTPUT

AC_KRB_VERSION(arla)
