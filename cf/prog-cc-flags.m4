dnl
dnl $Id: prog-cc-flags.m4,v 1.4 2004/02/12 16:28:18 lha Exp $
dnl

AC_DEFUN([AC_PROG_CC_FLAGS], [
AC_REQUIRE([AC_PROG_CC])dnl
AC_MSG_CHECKING(for $CC warning options)
if test "$GCC" = "yes"; then
dnl -Wbad-function-cast (is probably not useful)
  extra_flags="-Wall -Wmissing-prototypes -Wpointer-arith -Wmissing-declarations -Wnested-externs"
  CFLAGS="$CFLAGS $extra_flags"
  AC_MSG_RESULT($extra_flags)
else
  AC_MSG_RESULT(none)
fi
])
