dnl
dnl $Id: subst-val.m4,v 1.2 2004/02/12 16:28:18 lha Exp $
dnl

dnl arla_AC_SUBST_VALUE(enviroment-variable,value)
AC_DEFUN([arla_AC_SUBST_VALUE],[
$1="$2"
AC_SUBST($1)])
