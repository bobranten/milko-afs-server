dnl
dnl $Id: type-iovec.m4,v 1.3 2004/02/12 16:28:18 lha Exp $
dnl

dnl
dnl Check for struct iovec
dnl

AC_DEFUN([AC_TYPE_IOVEC], [

AC_CACHE_CHECK(for struct iovec, ac_cv_struct_iovec, [
AC_EGREP_HEADER(
changequote(, )dnl
struct[ 	]*iovec,
changequote([,])dnl
sys/uio.h,
ac_cv_struct_iovec=yes,
ac_cv_struct_iovec=no)
])
if test "$ac_cv_struct_iovec" = "yes"; then
  AC_DEFINE(HAVE_STRUCT_IOVEC, 1, [define if you have struct iovec])
fi
])
