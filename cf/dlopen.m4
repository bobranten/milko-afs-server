dnl
dnl $Id: dlopen.m4,v 1.1.1.1 2002/09/12 16:22:45 lha Exp $
dnl

AC_DEFUN([rk_DLOPEN], [
	AC_FIND_FUNC_NO_LIBS(dlopen, dl)
	AM_CONDITIONAL(HAVE_DLOPEN, test "$ac_cv_funclib_dlopen" != no)
])
