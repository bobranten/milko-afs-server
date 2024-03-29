dnl
dnl $Id: func-krb-get-default-tkt-root.m4,v 1.5 2004/02/12 16:28:16 lha Exp $
dnl

dnl
dnl Check for krb_get_default_tkt_root
dnl

AC_DEFUN([AC_FUNC_KRB_GET_DEFAULT_TKT_ROOT], [

AC_CACHE_CHECK(for krb_get_default_tkt_root, ac_cv_func_krb_get_default_tkt_root, [
if test "$ac_cv_found_krb4" = "yes"; then
save_CPPFLAGS="${CPPFLAGS}"
save_LIBS="${LIBS}"
CPPFLAGS="${KRB4_INC_FLAGS} ${CPPFLAGS}"
LIBS="${KRB4_LIB_FLAGS} ${LIBS}"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <krb.h>]], [[krb_get_default_tkt_root();]])],[ac_cv_func_krb_get_default_tkt_root=yes],[ac_cv_func_krb_get_default_tkt_root=no])
CPPFLAGS="${save_CPPFLAGS}"
LIBS="${save_LIBS}"
else
ac_cv_func_krb_get_default_tkt_root=no
fi
])
if test "$ac_cv_func_krb_get_default_tkt_root" = "yes"; then
  AC_DEFINE(HAVE_KRB_GET_DEFAULT_TKT_ROOT, 1, [define if you have krb_get_default_tkt_root])
fi
])
