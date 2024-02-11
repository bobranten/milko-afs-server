dnl
dnl $Id: kafs-settoken-rxkad.m4,v 1.7 2004/09/12 20:31:52 lha Exp $
dnl

dnl
dnl Check for kafs_settoken_rxkad
dnl

AC_DEFUN([AC_FUNC_KAFS_SETTOKEN_RXKAD], [

AC_CACHE_CHECK(for kafs_settoken_rxkad, ac_cv_func_kafs_settoken_rxkad, [
if test "$ac_cv_found_krb5" = "yes"; then
save_CPPFLAGS="${CPPFLAGS}"
save_LIBS="${LIBS}"
CPPFLAGS="${KAFS_CPPFLAGS} ${KRB5_INC_FLAGS} ${KRB4_INC_FLAGS} ${CPPFLAGS}"
LIBS="${KAFS_LIBS} ${LIBS}"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <kafs.h>]], [[kafs_settoken_rxkad(0,0,0,0);]])],[ac_cv_func_kafs_settoken_rxkad=yes],[ac_cv_func_kafs_settoken_rxkad=no])
CPPFLAGS="${save_CPPFLAGS}"
LIBS="${save_LIBS}"
else
ac_cv_func_kafs_settoken_rxkad=no
fi
])
if test "$ac_cv_func_kafs_settoken_rxkad" = "yes"; then
  AC_DEFINE(HAVE_KAFS_SETTOKEN_RXKAD, 1, [define if you have kafs_settoken_rxkad])
fi
])
