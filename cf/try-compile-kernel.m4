dnl
dnl $Id: try-compile-kernel.m4,v 1.4 2004/02/12 16:28:18 lha Exp $
dnl

AC_DEFUN([AC_TRY_COMPILE_KERNEL],[
save_CFLAGS="$CFLAGS"
save_CC="$CC"
if test "X${KERNEL_CC}" != "X"; then
  CC="$KERNEL_CC"
fi
CFLAGS="$CFLAGS $test_KERNEL_CFLAGS $KERNEL_CPPFLAGS"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[$1]], [[$2]])],[$3],[$4])
CFLAGS="$save_CFLAGS"
CC="$save_CC"
])
