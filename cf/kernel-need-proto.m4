dnl $Id: kernel-need-proto.m4,v 1.2 2004/02/12 16:28:17 lha Exp $
dnl
dnl
dnl Check if we need the prototype for a function in kernel-space
dnl

dnl AC_KERNEL_NEED_PROTO(includes, function)

AC_DEFUN([AC_KERNEL_NEED_PROTO], [
save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $test_KERNEL_CFLAGS $KERNEL_CPPFLAGS"
AC_NEED_PROTO([$1],[$2])
CFLAGS="$save_CFLAGS"
])
