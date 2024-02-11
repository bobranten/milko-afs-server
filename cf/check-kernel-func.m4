dnl
dnl $Id: check-kernel-func.m4,v 1.5 2002/10/28 10:58:20 haba Exp $
dnl

dnl AC_CHECK_KERNEL_FUNC(func, param, [includes])
AC_DEFUN([AC_CHECK_KERNEL_FUNC],
[AC_CHECK_KERNEL([$1], [ac_cv_kernel_func_$1], [[$1]([$2])], [$4])
if false; then
	AC_CHECK_FUNCS(patsubst([$1], [\(\w\|\_\)+], [kernel_\&]))
fi
])
