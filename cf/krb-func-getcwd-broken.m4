dnl $Id: krb-func-getcwd-broken.m4,v 1.5 2004/02/12 16:28:17 lha Exp $
dnl
dnl
dnl test for broken getcwd in (SunOS braindamage)
dnl

AC_DEFUN([AC_KRB_FUNC_GETCWD_BROKEN], [
if test "$ac_cv_func_getcwd" = yes; then
AC_MSG_CHECKING(if getcwd is broken)
AC_CACHE_VAL(ac_cv_func_getcwd_broken, [
ac_cv_func_getcwd_broken=no

AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <errno.h>
char *getcwd(char*, int);

void *popen(char *cmd, char *mode)
{
	errno = ENOTTY;
	return 0;
}

int main()
{
	char *ret;
	ret = getcwd(0, 1024);
	if(ret == 0 && errno == ENOTTY)
		return 0;
	return 1;
}
]])],[ac_cv_func_getcwd_broken=yes],[:],[:])
])
if test "$ac_cv_func_getcwd_broken" = yes; then
	AC_DEFINE(BROKEN_GETCWD, 1, [Define if getcwd is broken (like in SunOS 4).])dnl
	AC_LIBOBJ(getcwd)
	AC_MSG_RESULT($ac_cv_func_getcwd_broken)
else
	AC_MSG_RESULT([seems ok])
fi
fi
])
