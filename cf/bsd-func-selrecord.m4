dnl
dnl $Id: bsd-func-selrecord.m4,v 1.4 2004/02/12 16:28:15 lha Exp $
dnl

AC_DEFUN([AC_BSD_FUNC_SELRECORD], [
AC_CACHE_CHECK(if selrecord takes three arguments,
ac_cv_func_selrecord_three_args,
AC_TRY_COMPILE_KERNEL([
#ifdef HAVE_SYS_CDEFS_H
#include <sys/cdefs.h>
#endif
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/select.h>
#ifdef HAVE_SYS_SELINFO_H
#include <sys/selinfo.h>
#endif
], [selrecord(NULL, NULL, NULL)],
ac_cv_func_selrecord_three_args=yes,
ac_cv_func_selrecord_three_args=no))
if test "$ac_cv_func_selrecord_three_args" = yes; then
	AC_DEFINE(HAVE_THREE_ARGUMENT_SELRECORD, 1,
	[define if selrecord takes three arguments])
fi
])
