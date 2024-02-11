dnl
dnl $Id: check-kernel-var.m4,v 1.5 2002/10/28 11:05:15 haba Exp $
dnl

dnl AC_CHECK_KERNEL_VAR(var, type, [includes])
AC_DEFUN([AC_CHECK_KERNEL_VAR],
[AC_CHECK_KERNEL($1, ac_cv_kernel_var_$1, [extern $2 $1; return $1], $4)
if false; then
	AC_CHECK_FUNCS(patsubst([$1], [\(\w\|\_\)+], [kernel_\&]))
fi
])
