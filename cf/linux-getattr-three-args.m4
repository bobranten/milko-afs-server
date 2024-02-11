dnl
dnl $Id: linux-getattr-three-args.m4,v 1.3 2004/05/29 22:20:03 tol Exp $
dnl

AC_DEFUN([AC_LINUX_GETATTR_THREE_ARGS], [
AC_CACHE_CHECK(if getattr in struct inode_operations takes three args,
ac_cv_member_inode_operations_getattr_three_args,
AC_TRY_COMPILE_KERNEL([#include <asm/current.h>
#include <linux/fs.h>],
[
struct inode_operations io;
io.getattr(0,0);
],
ac_cv_member_inode_operations_getattr_three_args=no,
ac_cv_member_inode_operations_getattr_three_args=yes))

if test "$ac_cv_member_inode_operations_getattr_three_args" = "yes"; then
  AC_DEFINE(HAVE_GETATTR_THREE_ARGS, 1,
	[define if getattr in struct inode_operations takes three args])
fi
])
