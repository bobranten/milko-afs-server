dnl
dnl $Id: have-linux-kernel-types.m4,v 1.6 2004/02/12 16:28:17 lha Exp $
dnl
dnl Check for types in the Linux kernel
dnl

AC_DEFUN([AC_HAVE_LINUX_KERNEL_TYPES], [
for i in $1; do
        AC_HAVE_LINUX_KERNEL_TYPE($i)
done
if false; then
	AC_CHECK_FUNCS(patsubst([$1], [\(\w\|\_\)+], [linux_kernel_\&]))
fi
])
