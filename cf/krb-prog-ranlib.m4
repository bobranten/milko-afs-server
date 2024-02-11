dnl $Id: krb-prog-ranlib.m4,v 1.4 2004/02/12 16:28:17 lha Exp $
dnl
dnl
dnl Also look for EMXOMF for OS/2
dnl

AC_DEFUN([AC_KRB_PROG_RANLIB],
[AC_CHECK_PROGS(RANLIB, ranlib EMXOMF, :)])
