dnl
dnl $Id: check-kernel-funcs.m4,v 1.5 2002/10/28 11:02:03 haba Exp $
dnl

dnl AC_CHECK_KERNEL_FUNCS(functions...)
AC_DEFUN([AC_CHECK_KERNEL_FUNCS],
[for ac_func in $1
 do
 AC_CHECK_KERNEL($ac_func, ac_cv_kernel_func_$ac_func, [$ac_func]())
 done
 if false; then
	AC_CHECK_FUNCS(patsubst([$1], [\(\w\|\_\)+], [kernel_\&]))
 fi
])
