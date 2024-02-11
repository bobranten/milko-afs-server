dnl
dnl $Id: bsd-func-vfs-getnewfsid.m4,v 1.3 2004/02/12 16:28:15 lha Exp $
dnl

AC_DEFUN([AC_BSD_FUNC_VFS_GETNEWFSID], [
AC_CHECK_KERNEL_FUNCS(vfs_getnewfsid)
if test "$ac_cv_kernel_func_vfs_getnewfsid" = "yes"; then
AC_CACHE_CHECK(if vfs_getnewfsid takes two arguments,
ac_cv_func_vfs_getnewfsid_two_args,
AC_TRY_COMPILE_KERNEL([
#include <sys/types.h>
#include <sys/param.h>
#ifdef HAVE_SYS_MODULE_H
#include <sys/module.h>
#endif
#include <sys/mount.h>
#include <sys/vnode.h>
],
[vfs_getnewfsid(NULL, 0);],
ac_cv_func_vfs_getnewfsid_two_args=yes,
ac_cv_func_vfs_getnewfsid_two_args=no))
if test "$ac_cv_func_vfs_getnewfsid_two_args" = yes; then
	AC_DEFINE(HAVE_TWO_ARGUMENT_VFS_GETNEWFSID, 1,
	[define if vfs_getnewfsid takes two arguments])
fi
fi
])
