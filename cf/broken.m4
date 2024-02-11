dnl $Id: broken.m4,v 1.4 2002/09/12 16:24:51 lha Exp $
dnl
dnl
dnl Same as AC _REPLACE_FUNCS, just define HAVE_func if found in normal
dnl libraries 

AC_DEFUN([AC_BROKEN],
[m4_foreach_w([rk_func],[$1],[AC_CHECK_FUNC(rk_func,
		[AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_[]rk_func), 1, 
			[Define if you have the function `]rk_func['.])],
		[rk_LIBOBJ(rk_func)])])])
