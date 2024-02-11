dnl $Id: werror.m4,v 1.2 2004/02/12 16:28:18 lha Exp $
dnl
dnl Turn on Werror if we have gcc
dnl

dnl AC_WERROR(variable_name)

AC_DEFUN([AC_WERROR], [
if test X"$GCC" = Xyes ; then
  $1=-Werror
fi   
AC_SUBST($1)
])
