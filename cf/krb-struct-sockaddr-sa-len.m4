dnl $Id: krb-struct-sockaddr-sa-len.m4,v 1.2 2004/02/12 16:28:17 lha Exp $
dnl
dnl
dnl Check for sa_len in sys/socket.h
dnl

AC_DEFUN([AC_KRB_STRUCT_SOCKADDR_SA_LEN], [
AC_MSG_CHECKING(for sa_len in struct sockaddr)
AC_CACHE_VAL(ac_cv_struct_sockaddr_sa_len, [
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <sys/socket.h>]], [[struct sockaddr sa;
int foo = sa.sa_len;]])],[ac_cv_struct_sockaddr_sa_len=yes],[ac_cv_struct_sockaddr_sa_len=no])
])
if test "$ac_cv_struct_sockaddr_sa_len" = yes; then
	AC_DEFINE(SOCKADDR_HAS_SA_LEN)dnl
fi
AC_MSG_RESULT($ac_cv_struct_sockaddr_sa_len)
])
