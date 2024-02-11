dnl
dnl $Id: func-hstrerror-const.m4,v 1.3 2004/02/12 16:28:16 lha Exp $
dnl
dnl Test if hstrerror wants const or not
dnl

dnl AC_FUNC_HSTRERROR_CONST(includes, function)

AC_DEFUN([AC_FUNC_HSTRERROR_CONST], [
AC_CACHE_CHECK([if hstrerror needs const], ac_cv_func_hstrerror_const,
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[netdb.h]], [[const char *hstrerror(int);]])],[ac_cv_func_hstrerror_const=no],[ac_cv_func_hstrerror_const=yes]))
if test "$ac_cv_func_hstrerror_const" = "yes"; then
	AC_DEFINE(NEED_HSTRERROR_CONST, 1, [define if hstrerror is const])
fi
])
