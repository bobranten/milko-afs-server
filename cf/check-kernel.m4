dnl
dnl $Id: check-kernel.m4,v 1.15 2004/07/20 11:24:08 lha Exp $
dnl

dnl there are two different heuristics for doing the kernel tests
dnl a) running nm and greping the output
dnl b) trying linking against the kernel

dnl AC_CHECK_KERNEL(name, cv, magic, [includes])
AC_DEFUN([AC_CHECK_KERNEL],
[AC_MSG_CHECKING([for $1 in kernel])
AC_CACHE_VAL([$2],
[
if expr "$target_os" : "darwin" > /dev/null 2>&1; then
  if nm $KERNEL | egrep "\\<_?$1\\>" >/dev/null 2>&1; then
    eval "$2=yes"
  else
    eval "$2=no"
  fi
elif expr "$target_os" : "osf" >/dev/null 2>&1; then
  if nm  $KERNEL | egrep "^$1 " > /dev/null 2>&1; then
    eval "$2=yes"
  else
    eval "$2=no"
  fi
elif expr "$target_os" : "freebsd" >/dev/null 2>&1; then
  if nm  $KERNEL | egrep "T $1" > /dev/null 2>&1; then
    eval "$2=yes"
  else
    eval "$2=no"
  fi
elif expr "$target_os" : "dragonfly" >/dev/null 2>&1; then
  if nm  $KERNEL | egrep "T $1" > /dev/null 2>&1; then
    eval "$2=yes"
  else
    eval "$2=no"
  fi
elif expr "$target_os" : "netbsd" >/dev/null 2>&1; then
  if nm  $KERNEL | egrep "T _?$1" > /dev/null 2>&1; then
    eval "$2=yes"
  else
    eval "$2=no"
  fi
else
cat > conftest.$ac_ext <<EOF
dnl This sometimes fails to find confdefs.h, for some reason.
dnl [#]line __oline__ "[$]0"
[#]line __oline__ "configure"
#include "confdefs.h"
$4
int _foo() {
return foo();
}
int foo() {
$3;
return 0; }
EOF
save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $test_KERNEL_CFLAGS $KERNEL_CPPFLAGS"
if AC_TRY_EVAL(ac_compile) && AC_TRY_EVAL(ac_kernel_ld) && test -s conftest; then
  eval "$2=yes"
else
  eval "$2=no"
  echo "configure: failed program was:" >&AS_MESSAGE_LOG_FD
  cat conftest.$ac_ext >&AS_MESSAGE_LOG_FD
fi
CFLAGS="$save_CFLAGS"
rm -f conftest*
fi])

eval ac_res=\$$2
AC_MSG_RESULT($ac_res)
if test "$ac_res" = yes; then
  foo=HAVE_KERNEL_[]upcase($1)
  AC_DEFINE_UNQUOTED($foo, 1)
fi
])
