dnl $Id: have-type.m4,v 1.7 2004/02/12 16:28:17 lha Exp $
dnl
dnl check for existance of a type

dnl AC_HAVE_TYPE(TYPE,INCLUDES)
AC_DEFUN([AC_HAVE_TYPE], [
cv=`echo "$1" | sed 'y%./+- %__p__%'`
AC_MSG_CHECKING(for $1)
AC_CACHE_VAL([ac_cv_type_$cv],
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
$2]], [[$1 foo;]])],[eval "ac_cv_type_$cv=yes"],[eval "ac_cv_type_$cv=no"]))dnl
ac_foo=`eval echo \\$ac_cv_type_$cv`
AC_MSG_RESULT($ac_foo)
if test "$ac_foo" = yes; then
  ac_tr_hdr=HAVE_`echo $1 | sed 'y%abcdefghijklmnopqrstuvwxyz./- %ABCDEFGHIJKLMNOPQRSTUVWXYZ____%'`
if false; then
	AC_CHECK_TYPES($1)
fi
  AC_DEFINE_UNQUOTED($ac_tr_hdr, 1, [Define if you have type `$1'])
fi
])
