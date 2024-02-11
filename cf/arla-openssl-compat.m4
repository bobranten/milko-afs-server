dnl $Id: arla-openssl-compat.m4,v 1.2 2004/02/12 16:28:15 lha Exp $
dnl
AC_DEFUN([arla_OPENSSL_COMPAT],
[AH_TOP([#ifndef OPENSSL_DES_LIBDES_COMPATIBILITY
#define OPENSSL_DES_LIBDES_COMPATIBILITY 1
#endif])])
