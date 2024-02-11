dnl $Id: arla-canonical.m4,v 1.2 2004/02/12 16:28:15 lha Exp $
dnl Modern autoconf doesn't AC_SUBST target_\* variables gree
dnl
AC_DEFUN([arla_CANONICAL],[
AC_CANONICAL_TARGET
AC_SUBST(target_cpu)dnl
AC_SUBST(target_vendor)dnl
AC_SUBST(target_os)dnl
])
