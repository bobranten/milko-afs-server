dnl $Id: broken-getnameinfo.m4,v 1.4 2004/02/12 16:28:15 lha Exp $
dnl
dnl test for broken AIX getnameinfo

AC_DEFUN([rk_BROKEN_GETNAMEINFO],[
AC_CACHE_CHECK([if getnameinfo is broken], ac_cv_func_getnameinfo_broken,
AC_RUN_IFELSE([AC_LANG_SOURCE([[[#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int
main(int argc, char **argv)
{
  struct sockaddr_in sin;
  char host[256];
  memset(&sin, 0, sizeof(sin));
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
  sin.sin_len = sizeof(sin);
#endif
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = 0xffffffff;
  sin.sin_port = 0;
  return getnameinfo((struct sockaddr*)&sin, sizeof(sin), host, sizeof(host),
	      NULL, 0, 0);
}
]]])],[ac_cv_func_getnameinfo_broken=no],[ac_cv_func_getnameinfo_broken=yes],[]))])
