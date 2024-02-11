dnl
dnl $Id: check-sl.m4,v 1.8 2005/11/24 13:35:55 tol Exp $
dnl

AC_DEFUN([AC_CHECK_SL],[

SL_H=sl.h
DIR_sl=sl
LIB_sl='$(top_builddir)/lib/sl/libsl.la'
INC_sl='-I$(top_builddir)/include'
DEPEND_sl='$(top_builddir)/lib/sl/libsl.la'

AC_ARG_WITH(sl,
[  --with-sl=dir           make with sl in dir],
[if test "$with_sl" != "no"; then
   SL_H=
   DIR_sl=
   DEPEND_sl=
   if test "X$withval" != "Xyes"; then
	if test -f "$withval/lib/liblsl.la" ; then
	    LIB_sl="$withval/lib/liblsl.la"
	else
   	    LIB_sl="-L$withval/lib -lsl"
	fi
   	INC_sl="-I$withval/include"
   else
	LIB_sl='-lsl'
	INC_sl=
   fi
fi],[with_sl=builtin])

AC_ARG_WITH(sl-include,
[  --with-sl-include=dir   make with sl headers in dir],
[if test "$with_sl" != "no"; then
   SL_H=
   DIR_sl=
   if test "X$withval" != "Xyes"; then
   	INC_sl="-I$withval"
   else
	INC_sl=
   fi
fi])

AC_ARG_WITH(sl-lib,
[  --with-sl-lib=dir       make with sl lib in dir],
[if test "$with_sl" != "no"; then
   SL_H=
   DIR_sl=
   DEPEND_sl=
   if test "X$withval" != "Xyes"; then
	if test -f "$withval/liblsl.la" ; then
	    LIB_sl="$withval/liblsl.la"
	else
   	    LIB_sl="-L$withval -lsl"
	fi
   else
   	LIB_sl="-lsl"
   fi
fi])

dnl kerberos may include sl, lets use that if available
dnl should this try the roken path instead?

if test "X$SL_H" != "X"; then

  old_LIBS="$LIBS"
  LIBS="$KRB5_LIB_DIR -lsl $LIB_readline_ac $LIBS"

  AC_MSG_CHECKING([for sl in $KRB5_LIB_DIR])

  AC_TRY_LINK_FUNC(sl_apropos,
  [working_krb_sl_libs=yes],[working_krb_sl_libs=no])
  AC_MSG_RESULT($working_krb_sl_libs)

  LIBS="$old_LIBS"

  if test "X$working_krb_sl_libs" = "Xyes"; then
    SL_H=
    DIR_sl=
    DEPEND_sl=
    LIB_sl="$KRB5_LIB_DIR -lsl"
    INC_sl="$KRB5_INC_DIR"
  fi

fi

AC_SUBST(INC_sl)
AC_SUBST(LIB_sl)
AC_SUBST(DIR_sl)
AC_SUBST(SL_H)
AC_SUBST(DEPEND_sl)

AC_MSG_CHECKING([if sl have sl_apropos])
AC_CACHE_VAL(ac_cv_slcompat_sl_apropos,[

if test "X$with_sl" = "Xbuiltin"; then
    dnl built in sl do have sl_apropos
    ac_cv_slcompat_sl_apropos=builtin 	
else
    old_LIBS="$LIBS"
    LIBS="$LIB_sl $LIB_readline_ac $LIBS"
    AC_TRY_LINK_FUNC(sl_apropos,
    [ac_cv_slcompat_sl_apropos=yes],[ac_cv_slcompat_sl_apropos=no])

    LIBS="$old_LIBS"
fi
])

if test "X$ac_cv_slcompat_sl_apropos" = "Xno"; then
	AC_DEFINE_UNQUOTED(NEED_SLCOMPAT_SL_APROPOS, 1,[libsl need sl_apropos])
fi

AC_MSG_RESULT($ac_cv_slcompat_sl_apropos)

])