# $Id: Makefile.am,v 1.6 2005/08/25 08:45:22 lha Exp $

include $(top_srcdir)/Makefile.am.common

AM_CPPFLAGS += \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/appl/arlalib \
	-I../../rxdef \
	-I$(srcdir)/../..

bin_PROGRAMS = tokens unlog

man_MANS = aklog.1 tokens.1 unlog.1

tokens_SOURCES = tokens.c
unlog_SOURCES = unlog.c

LDADD = ../../appl/arlalib/libarlalib.la \
	../../rxdef/librxdefclient.la \
	../../lib/ko/libko.la \
	../../lib/util/libarlautil.la \
	../../rx/librx.la \
	../../lwp/liblwp.a \
	../../lib/vers/libvers.la \
	$(RXKAD_LIBS) \
	$(KAFS_LIBS) \
	$(LIB_sl) \
	$(LIB_roken)
