dnl
dnl $Id: krb-irix.m4,v 1.3 2004/02/12 16:28:17 lha Exp $
dnl

dnl requires AC_CANONICAL_HOST
AC_DEFUN([KRB_IRIX],[
irix=no
case "$host_os" in
irix*) irix=yes ;;
esac
AM_CONDITIONAL(IRIX, test "$irix" != no)dnl
])
