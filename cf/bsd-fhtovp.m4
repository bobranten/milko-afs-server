dnl
dnl $Id: bsd-fhtovp.m4,v 1.1 2006/02/07 14:42:18 lha Exp $
dnl

dnl
dnl Find out if vfs_fhtovp takes three arguments
dnl

AC_DEFUN([AC_BSD_FHTOVP], [
AC_CACHE_CHECK(if vfs_fhtovp takes three arguments, ac_cv_kern_func_vfs_fhtovp,
AC_TRY_COMPILE_KERNEL([
#ifdef HAVE_SYS_CDEFS_H
#include <sys/cdefs.h>
#endif
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/vnode.h>
#include <sys/mount.h>
],[struct vfsops ops;
ops.vfs_fhtovp(0,0,0);],
ac_cv_kern_func_vfs_fhtovp=yes,
ac_cv_kern_func_vfs_fhtovp=no))
if test "$ac_cv_kern_func_vfs_fhtovp" = yes; then
	AC_DEFINE_UNQUOTED(HAVE_THREE_ARGUMENT_FHTOVP, 1,
	[define if vfs_fhtovp takes three arguments])
fi

])
