dnl
dnl $Id: check-roken.m4,v 1.8 2005/11/24 13:36:45 tol Exp $
dnl

AC_DEFUN([AC_CHECK_ROKEN],[

ROKEN_H=roken.h
DIR_roken=roken
LIB_roken='$(top_builddir)/lib/roken/libroken.a'
INC_roken='-I$(top_builddir)/include'
ac_cv_arla_with_roken=yes


AC_ARG_WITH(roken,
[  --with-roken=dir        make with roken in dir],
[if test "$with_roken" != "no"; then
   ROKEN_H=
   DIR_roken=
   if test "X$withval" != "Xyes"; then
   	LIB_roken="-L$withval/lib -lroken"
   	INC_roken="-I$withval/include"

   else
	LIB_roken='-lroken'
	INC_roken=
   fi
   ac_cv_arla_with_roken=no
fi])

AC_ARG_WITH(roken-include,
[  --with-roken-include=dir make with roken headers in dir],
[if test "$with_roken" != "no"; then
   ROKEN_H=
   DIR_roken=
   if test "X$withval" != "Xyes"; then
   	INC_roken="-I$withval"
   else
	INC_roken=
   fi
   ac_cv_arla_with_roken=no
fi])

AC_ARG_WITH(roken-lib,
[  --with-roken-lib=dir    make with roken lib in dir],
[if test "$with_roken" != "no"; then
   ROKEN_H=
   DIR_roken=
   if test "X$withval" != "X"; then
   	LIB_roken="-L$withval -lroken"
   else
   	LIB_roken="-lroken"
   fi
   ac_cv_arla_with_roken=no
fi])

dnl kerberos may include a roken, lets use that if available

if test "X$ac_cv_arla_with_roken" = "Xyes"; then
  if expr "x$KRB5_LIB_FLAGS" : ".*-lroken" > /dev/null ; then

    old_CFLAGS="$CFLAGS"
    CFLAGS="$KRB5_INC_DIR $CFLAGS"

    AC_CHECK_HEADERS([getarg.h],
    [working_krb_getarg=yes],[working_krb_getarg=no])

    CFLAGS="$old_CFLAGS"

    if test "X$working_krb_getarg" = "Xyes"; then
      ROKEN_H=
      DIR_roken=
      LIB_roken="$KRB5_LIB_DIR -lroken"
      INC_roken="$KRB5_INC_DIR"
      ac_cv_arla_with_roken=no
    fi
  fi
fi

if test "X$ROKEN_H" = "X"; then

AC_CACHE_CHECK([what roken depends on ],[ac_cv_roken_deps],[
ac_cv_roken_deps="error"
saved_LIBS="$LIBS"
for a in "" "-lcrypt" ; do
  LIBS="$saved_LIBS $LIB_roken $a"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[getarg()]])],[
    if test "X$a" = "X"; then
      ac_cv_roken_deps="nothing"
    else
      ac_cv_roken_deps="$a"
    fi],[])
  LIBS="$saved_LIBS"
  if test $ac_cv_roken_deps != "error"; then break; fi
done
LIBS="$saved_LIBS"
])

if test "$ac_cv_roken_deps" = "error"; then
  AC_MSG_ERROR([failed to figure out libroken depencies])
fi

if test "$ac_cv_roken_deps" != "nothing"; then
  LIB_roken="$LIB_roken $ac_cv_roken_deps"
fi

fi

AC_SUBST(INC_roken)
AC_SUBST(LIB_roken)
AC_SUBST(DIR_roken)
AC_SUBST(ROKEN_H)

])
