dnl
dnl $Id: bsd-header-vnode-if-h.m4,v 1.5 2007/03/06 12:41:12 tol Exp $
dnl

AC_DEFUN([AC_BSD_HEADER_VNODE_IF_H], [
AC_MSG_CHECKING(if vnode_if.h needs to be built)
changequote(, )dnl
rm -f vnode_if.[ch]
changequote([,])dnl
AC_TRY_CPP_KERNEL([
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/vnode.h>
], ac_cv_header_vnode_if_h=no,
ac_cv_header_vnode_if_h=yes)
dnl
if test "$ac_cv_header_vnode_if_h" = "yes"; then
test -d ../sys || mkdir ../sys 
if test -f $SYS/kern/vnode_if.pl; then
  perl $SYS/kern/vnode_if.pl -h $SYS/kern/vnode_if.src
elif test -f $SYS/kern/vnode_if.sh; then
  /bin/sh $SYS/kern/vnode_if.sh $SYS/kern/vnode_if.src
elif test -f $SYS/tools/vnode_if.awk ; then
  awk -f $SYS/tools/vnode_if.awk $SYS/kern/vnode_if.src -h
  awk -f $SYS/tools/vnode_if.awk $SYS/kern/vnode_if.src -p
  awk -f $SYS/tools/vnode_if.awk $SYS/kern/vnode_if.src -q
else
  AC_MSG_ERROR(unable to find any vnode_if script)
fi
if test -f vnode_if.h; then
  :
else
  AC_MSG_ERROR(failed to create vnode_if.h)
fi
AC_TRY_CPP_KERNEL([
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/vnode.h>
], ac_cv_header_vnode_if_h=yes; VNODE_IF_H=vnode_if.h,
AC_MSG_ERROR(tried creating vnode_if.h but still could not include vnode.h))
fi
AC_MSG_RESULT($ac_cv_header_vnode_if_h)
AC_SUBST(VNODE_IF_H)])
